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

            if (evaluateCondition(ifNode->condition(), context)) {
                return renderNodes(ifNode->trueBranch(), context);
            }

            return renderNodes(ifNode->falseBranch(), context);
        }

        case NodeType::Foreach: {
            const auto foreachNode =
                std::dynamic_pointer_cast<ForeachNode>(node);

            const auto expression =
                parseForeachExpression(foreachNode->expression());

            if (!expression.has_value()) {
                return "";
            }

            const auto makeLoop = [](std::size_t index, std::size_t count) {
                Json::Value loop(Json::objectValue);
                loop["index"] = static_cast<Json::UInt64>(index);
                loop["number"] = static_cast<Json::UInt64>(index + 1);
                loop["first"] = index == 0;
                loop["last"] = index + 1 == count;
                loop["count"] = static_cast<Json::UInt64>(count);

                return loop;
            };

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

                std::string output;
                for (std::size_t renderedIndex = 0;
                     renderedIndex < selected.size();
                     ++renderedIndex
                ) {
                    auto childContext = context.createChild();
                    childContext.set(
                        expression->variable,
                        (*stringValues)[selected[renderedIndex]]
                    );
                    childContext.set("loop", makeLoop(renderedIndex, selected.size()));
                    output += renderNodes(foreachNode->body(), childContext);
                }
                return output;
            }

            const auto collection =
                resolveJsonValue(expression->collection, context);

            if (!collection.has_value() || !collection->isArray()) {
                return "";
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

            std::string output;
            for (std::size_t index = 0; index < selected.size(); ++index) {
                auto childContext = context.createChild();
                childContext.set(expression->variable, selected[index]);
                childContext.set("loop", makeLoop(index, selected.size()));
                output += renderNodes(foreachNode->body(), childContext);
            }
            return output;
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