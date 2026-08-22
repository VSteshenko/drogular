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

            std::string output;

            if (const auto stringValues =
                context.get<std::vector<std::string>>(expression->collection)) {
                std::vector<std::size_t> selected;
                selected.reserve(stringValues->size());

                for (std::size_t index = 0; index < stringValues->size(); ++index) {
                    if (expression->condition.has_value()) {
                        auto conditionContext = context.createChild();
                        conditionContext.set(expression->variable, (*stringValues)[index]);
                        if (!evaluateCondition(*expression->condition, conditionContext)) {
                            continue;
                        }
                    }
                    selected.push_back(index);
                }

                if (selected.empty()) {
                    return renderNodes(foreachNode->emptyBranch(), context);
                }

                for (std::size_t renderedIndex = 0;
                     renderedIndex < selected.size();
                     ++renderedIndex
                ) {
                    auto childContext = context.createChild();
                    childContext.set(
                        expression->variable,
                        (*stringValues)[selected[renderedIndex]]
                    );
                    detail::setLoopMetadata(
                        childContext,
                        context,
                        renderedIndex,
                        selected.size()
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

            const auto collection =
                resolveJsonValue(expression->collection, context);

            if (!collection.has_value() || !collection->isArray()) {
                return renderNodes(foreachNode->emptyBranch(), context);
            }

            std::vector<Json::Value> selected;
            selected.reserve(collection->size());

            for (const auto& item : *collection) {
                if (expression->condition.has_value()) {
                    auto conditionContext = context.createChild();
                    conditionContext.set(expression->variable, item);
                    if (!evaluateCondition(*expression->condition, conditionContext)) {
                        continue;
                    }
                }
                selected.push_back(item);
            }

            if (selected.empty()) {
                return renderNodes(foreachNode->emptyBranch(), context);
            }

            for (std::size_t index = 0; index < selected.size(); ++index) {
                auto childContext = context.createChild();
                childContext.set(expression->variable, selected[index]);
                detail::setLoopMetadata(
                    childContext,
                    context,
                    index,
                    selected.size()
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