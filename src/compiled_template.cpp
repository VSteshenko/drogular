#include <drogular/compiled_template.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/template_parser.hpp>
#include <drogular/template_tokenizer.hpp>
#include <drogular/template_runtime.hpp>
#include <drogular/render_context.hpp>

#include <optional>

namespace drogular::template_compiler {

namespace {

std::string nodesToTemplate(
    const std::vector<NodePtr>& nodes
) {
    std::string output;

    for (const auto& node : nodes) {
        switch (node->type()) {
            case NodeType::Text: {
                const auto textNode =
                    std::dynamic_pointer_cast<TextNode>(node);

                output += textNode->text();
                break;
            }

            case NodeType::Variable: {
                const auto variableNode =
                    std::dynamic_pointer_cast<VariableNode>(node);

                output += "{{ " + variableNode->expression() + " }}";
                break;
            }

            case NodeType::RawVariable: {
                const auto rawNode =
                    std::dynamic_pointer_cast<RawVariableNode>(node);

                output += "{{{ " + rawNode->expression() + " }}}";
                break;
            }

            case NodeType::Component: {
                const auto componentNode =
                    std::dynamic_pointer_cast<ComponentNode>(node);

                output += componentNode->tagHtml();
                break;
            }

            case NodeType::If: {
                const auto ifNode =
                    std::dynamic_pointer_cast<IfNode>(node);

                output += "@if(" + ifNode->condition() + ")";
                output += nodesToTemplate(ifNode->trueBranch());

                if (!ifNode->falseBranch().empty()) {
                    output += "@else";
                    output += nodesToTemplate(ifNode->falseBranch());
                }

                output += "@endif";
                break;
            }

            case NodeType::Foreach: {
                const auto foreachNode =
                    std::dynamic_pointer_cast<ForeachNode>(node);

                output += "@foreach(" + foreachNode->expression() + ")";
                output += nodesToTemplate(foreachNode->body());

                if (!foreachNode->emptyBranch().empty()) {
                    output += "@empty";
                    output += nodesToTemplate(foreachNode->emptyBranch());
                }

                output += "@endforeach";
                break;
            }

            case NodeType::Break:
                output += "@break";
                break;

            case NodeType::Continue:
                output += "@continue";
                break;
        }
    }

    return output;
}

enum class RenderControl {
    None,
    Break,
    Continue
};

struct RenderResult {
    std::string output;
    RenderControl control = RenderControl::None;
};

RenderResult renderNodes(
    const std::vector<NodePtr>& nodes,
    RenderContext& context
);

RenderResult renderNode(
    const NodePtr& node,
    RenderContext& context
) {
    switch (node->type()) {
        case NodeType::Text: {
            const auto textNode =
                std::dynamic_pointer_cast<TextNode>(node);

            return { .output = textNode->text() };
        }

        case NodeType::Variable: {
            const auto variableNode =
                std::dynamic_pointer_cast<VariableNode>(node);

            return {
                .output = resolveVariable(
                    variableNode->expression(),
                    context
                ).value_or("")
            };
        }

        case NodeType::RawVariable: {
            const auto rawNode =
                std::dynamic_pointer_cast<RawVariableNode>(node);

            return {
                .output = resolveRawVariable(
                    rawNode->expression(),
                    context
                ).value_or("")
            };
        }

        case NodeType::If: {
            const auto ifNode =
                std::dynamic_pointer_cast<IfNode>(node);

            if (evaluateCondition(ifNode->condition(), context)) {
                return renderNodes(ifNode->trueBranch(), context);
            }

            return renderNodes(ifNode->falseBranch(), context);
        }

        case NodeType::Break:
            return { .control = RenderControl::Break };

        case NodeType::Continue:
            return { .control = RenderControl::Continue };

        case NodeType::Foreach: {
            const auto foreachNode =
                std::dynamic_pointer_cast<ForeachNode>(node);

            const auto expression =
                parseForeachExpression(foreachNode->expression());
            if (!expression.has_value()) {
                return {};
            }

            const auto source = template_expression::parse(expression->collection);
            if (!source) {
                return renderNodes(foreachNode->emptyBranch(), context);
            }

            const auto iterable = template_expression::evaluate(
                *source.expression,
                context
            ).iterable();
            if (iterable == nullptr || iterable->empty()) {
                return renderNodes(foreachNode->emptyBranch(), context);
            }

            std::vector<template_expression::ExpressionValue> selected;
            if (expression->condition.has_value()) {
                selected.reserve(iterable->size());
                for (std::size_t index = 0; index < iterable->size(); ++index) {
                    auto value = iterable->at(index);
                    auto conditionContext = context.createChild();
                    detail::setExpressionValue(
                        conditionContext,
                        expression->variable,
                        value
                    );
                    if (evaluateCondition(*expression->condition, conditionContext)) {
                        selected.push_back(std::move(value));
                    }
                }

                if (selected.empty()) {
                    return renderNodes(foreachNode->emptyBranch(), context);
                }
            }

            const auto count = expression->condition.has_value()
                ? selected.size()
                : iterable->size();
            std::string output;

            for (std::size_t index = 0; index < count; ++index) {
                auto childContext = context.createChild();
                auto value = expression->condition.has_value()
                    ? selected[index]
                    : iterable->at(index);
                detail::setExpressionValue(
                    childContext,
                    expression->variable,
                    std::move(value)
                );
                detail::setLoopMetadata(
                    childContext,
                    context,
                    index,
                    count
                );

                auto result = renderNodes(foreachNode->body(), childContext);
                output += result.output;

                if (result.control == RenderControl::Break) {
                    break;
                }
                if (result.control == RenderControl::Continue) {
                    continue;
                }
            }

            return { .output = std::move(output) };
        }

        case NodeType::Component: {
            const auto componentNode =
                std::dynamic_pointer_cast<ComponentNode>(node);

            if (context.services() == nullptr) {
                return { .output = componentNode->tagHtml() };
            }

            return {
                .output = component_renderer::render(
                    componentNode->tagHtml(),
                    context.services()->components(),
                    context
                )
            };
        }
    }

    return {};
}

RenderResult renderNodes(
    const std::vector<NodePtr>& nodes,
    RenderContext& context
) {
    RenderResult result;

    for (const auto& node : nodes) {
        auto nodeResult = renderNode(node, context);
        result.output += nodeResult.output;

        if (nodeResult.control != RenderControl::None) {
            result.control = nodeResult.control;
            break;
        }
    }

    return result;
}

} // namespace

CompiledTemplate::CompiledTemplate(std::vector<NodePtr> nodes)
    : nodes_(std::move(nodes)) {
}

std::string CompiledTemplate::render(RenderContext& context) const {
    return renderNodes(nodes_, context).output;
}

CompiledTemplate compile(std::string_view html) {
    TemplateDiagnostics diagnostics;

    return CompiledTemplate(
        parse(
            tokenize(html, diagnostics),
            diagnostics
        )
    );
}

CompileResult compileWithDiagnostics(
    std::string_view html,
    std::string sourceName
) {
    TemplateDiagnostics diagnostics(
        html,
        std::move(sourceName)
    );

    auto nodes =
        parse(
            tokenize(html, diagnostics),
            diagnostics
        );

    return {
        .compiledTemplate = CompiledTemplate(std::move(nodes)),
        .diagnostics = std::move(diagnostics)
    };
}

} // namespace drogular::template_compiler