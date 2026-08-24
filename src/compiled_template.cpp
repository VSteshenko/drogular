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

            case NodeType::Let:
            case NodeType::Const: {
                const auto bindingNode =
                    std::dynamic_pointer_cast<BindingNode>(node);

                output += node->type() == NodeType::Let ? "@let(" : "@const(";
                output += bindingNode->name() + " = " +
                    bindingNode->expression() + ")";
                break;
            }

            case NodeType::Switch: {
                const auto switchNode =
                    std::dynamic_pointer_cast<SwitchNode>(node);

                output += "@switch(" + switchNode->expression() + ")";
                for (const auto& switchCase : switchNode->cases()) {
                    output += "@case(" + switchCase.expressions + ")";
                    output += nodesToTemplate(switchCase.body);
                }
                if (!switchNode->defaultBranch().empty()) {
                    output += "@default";
                    output += nodesToTemplate(switchNode->defaultBranch());
                }
                output += "@endswitch";
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
    RenderContext& context,
    template_expression::BindingContext& bindings
);

RenderResult renderNode(
    const NodePtr& node,
    RenderContext& context,
    template_expression::BindingContext& bindings
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
                    bindings
                ).value_or("")
            };
        }

        case NodeType::RawVariable: {
            const auto rawNode =
                std::dynamic_pointer_cast<RawVariableNode>(node);

            return {
                .output = resolveRawVariable(
                    rawNode->expression(),
                    bindings
                ).value_or("")
            };
        }

        case NodeType::If: {
            const auto ifNode =
                std::dynamic_pointer_cast<IfNode>(node);

            auto branchBindings = bindings.createChild();
            if (ifNode->conditionExpression() != nullptr &&
                template_expression::evaluate(
                    *ifNode->conditionExpression(), bindings
                ).truthy()
            ) {
                return renderNodes(
                    ifNode->trueBranch(), context, branchBindings
                );
            }

            return renderNodes(
                ifNode->falseBranch(), context, branchBindings
            );
        }

        case NodeType::Let:
        case NodeType::Const: {
            const auto bindingNode =
                std::dynamic_pointer_cast<BindingNode>(node);
            if (bindingNode->compiledExpression() == nullptr) {
                return {};
            }

            bindings.define(
                bindingNode->name(),
                template_expression::evaluate(
                    *bindingNode->compiledExpression(), bindings
                ),
                bindingNode->mutability()
            );
            return {};
        }

        case NodeType::Break:
            return { .control = RenderControl::Break };

        case NodeType::Continue:
            return { .control = RenderControl::Continue };

        case NodeType::Switch: {
            const auto switchNode =
                std::dynamic_pointer_cast<SwitchNode>(node);

            if (switchNode->compiledExpression() == nullptr) {
                return {};
            }

            const auto switchValue = template_expression::evaluate(
                *switchNode->compiledExpression(), bindings
            );

            for (const auto& switchCase : switchNode->cases()) {
                if (switchCase.compiledExpressions == nullptr) {
                    continue;
                }

                const auto candidates = template_expression::evaluate(
                    *switchCase.compiledExpressions, bindings
                ).iterable();
                if (candidates == nullptr) {
                    continue;
                }

                bool matches = false;
                for (std::size_t index = 0; index < candidates->size(); ++index) {
                    if (switchValue.equals(candidates->at(index))) {
                        matches = true;
                        break;
                    }
                }

                if (matches) {
                    auto branchBindings = bindings.createChild();
                    return renderNodes(
                        switchCase.body, context, branchBindings
                    );
                }
            }

            auto defaultBindings = bindings.createChild();
            return renderNodes(
                switchNode->defaultBranch(), context, defaultBindings
            );
        }

        case NodeType::Foreach: {
            const auto foreachNode =
                std::dynamic_pointer_cast<ForeachNode>(node);

            if (foreachNode->collectionExpression() == nullptr) {
                auto emptyBindings = bindings.createChild();
                return renderNodes(
                    foreachNode->emptyBranch(), context, emptyBindings
                );
            }

            const auto iterable = template_expression::evaluate(
                *foreachNode->collectionExpression(),
                bindings
            ).iterable();
            if (iterable == nullptr || iterable->empty()) {
                auto emptyBindings = bindings.createChild();
                return renderNodes(
                    foreachNode->emptyBranch(), context, emptyBindings
                );
            }

            std::vector<template_expression::ExpressionValue> selected;
            if (foreachNode->conditionExpression() != nullptr) {
                selected.reserve(iterable->size());
                for (std::size_t index = 0; index < iterable->size(); ++index) {
                    auto value = iterable->at(index);
                    auto conditionBindings = bindings.createChild();
                    conditionBindings.define(
                        foreachNode->variable(),
                        value,
                        template_engine::BindingMutability::Mutable
                    );
                    if (template_expression::evaluate(
                            *foreachNode->conditionExpression(), conditionBindings
                        ).truthy()
                    ) {
                        selected.push_back(std::move(value));
                    }
                }

                if (selected.empty()) {
                    auto emptyBindings = bindings.createChild();
                    return renderNodes(
                        foreachNode->emptyBranch(), context, emptyBindings
                    );
                }
            }

            const auto count = foreachNode->conditionExpression() != nullptr
                ? selected.size()
                : iterable->size();
            std::string output;

            for (std::size_t index = 0; index < count; ++index) {
                auto childContext = context.createChild();
                auto value = foreachNode->conditionExpression() != nullptr
                    ? selected[index]
                    : iterable->at(index);
                detail::setExpressionValue(
                    childContext,
                    foreachNode->variable(),
                    value
                );
                detail::setLoopMetadata(
                    childContext,
                    context,
                    index,
                    count
                );

                auto bodyBindings = bindings.createChild();
                bodyBindings.define(
                    foreachNode->variable(),
                    value,
                    template_engine::BindingMutability::Mutable
                );
                if (const auto loop = childContext.get<Json::Value>("loop")) {
                    bodyBindings.define(
                        "loop",
                        template_expression::ExpressionValue(*loop),
                        template_engine::BindingMutability::Constant
                    );
                }

                auto result = renderNodes(
                    foreachNode->body(), childContext, bodyBindings
                );
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

            auto componentContext = context.createChild();
            bindings.materialize(componentContext);

            return {
                .output = component_renderer::render(
                    componentNode->tagHtml(),
                    context.services()->components(),
                    componentContext,
                    bindings
                )
            };
        }
    }

    return {};
}

RenderResult renderNodes(
    const std::vector<NodePtr>& nodes,
    RenderContext& context,
    template_expression::BindingContext& bindings
) {
    RenderResult result;

    for (const auto& node : nodes) {
        auto nodeResult = renderNode(node, context, bindings);
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
    template_expression::BindingContext bindings(context);
    return render(context, bindings);
}

std::string CompiledTemplate::render(
    RenderContext& context,
    template_expression::BindingContext& bindings
) const {
    return renderNodes(nodes_, context, bindings).output;
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