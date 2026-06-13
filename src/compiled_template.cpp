#include <drogular/compiled_template.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/template_engine.hpp>
#include <drogular/template_parser.hpp>
#include <drogular/template_tokenizer.hpp>
#include <drogular/template_runtime.hpp>

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
                output += "@endforeach";
                break;
            }
        }
    }

    return output;
}

std::string renderNodes(
    const std::vector<NodePtr>& nodes,
    RenderContext& context
);

std::string renderNode(
    const NodePtr& node,
    RenderContext& context
) {
    switch (node->type()) {
        case NodeType::Text: {
            const auto textNode =
                std::dynamic_pointer_cast<TextNode>(node);

            return textNode->text();
        }

        case NodeType::Variable: {
            const auto variableNode =
                std::dynamic_pointer_cast<VariableNode>(node);

            return resolveVariable(
                variableNode->expression(),
                context
            ).value_or("");
        }

        case NodeType::RawVariable: {
            const auto rawNode =
                std::dynamic_pointer_cast<RawVariableNode>(node);

            return resolveRawVariable(
                rawNode->expression(),
                context
            ).value_or("");
        }

        case NodeType::If: {
            const auto ifNode =
                std::dynamic_pointer_cast<IfNode>(node);

            const auto conditionHtml =
                template_engine::render(
                    "@if(" + ifNode->condition() + ")1@else0@endif",
                    context
                );

            if (conditionHtml == "1") {
                return renderNodes(ifNode->trueBranch(), context);
            }

            if (evaluateCondition(ifNode->condition(), context)) {
                return renderNodes(ifNode->trueBranch(), context);
            }

            return renderNodes(ifNode->falseBranch(), context);
        }

        case NodeType::Foreach: {
            const auto foreachNode =
                std::dynamic_pointer_cast<ForeachNode>(node);

            const auto html =
                "@foreach(" + foreachNode->expression() + ")" +
                nodesToTemplate(foreachNode->body()) +
                "@endforeach";

            return template_engine::render(html, context);
        }

        case NodeType::Component: {
            const auto componentNode =
                std::dynamic_pointer_cast<ComponentNode>(node);

            if (context.services() == nullptr) {
                return componentNode->tagHtml();
            }

            return component_renderer::render(
                componentNode->tagHtml(),
                context.services()->components(),
                context
            );
        }
    }

    return "";
}

std::string renderNodes(
    const std::vector<NodePtr>& nodes,
    RenderContext& context
) {
    std::string output;

    for (const auto& node : nodes) {
        output += renderNode(node, context);
    }

    return output;
}

} // namespace

CompiledTemplate::CompiledTemplate(std::vector<NodePtr> nodes)
    : nodes_(std::move(nodes)) {
}

std::string CompiledTemplate::render(RenderContext& context) const {
    return renderNodes(nodes_, context);
}

CompiledTemplate compile(std::string_view html) {
    return CompiledTemplate(
        parse(tokenize(html))
    );
}

} // namespace drogular::template_compiler