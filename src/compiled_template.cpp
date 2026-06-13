#include <drogular/compiled_template.hpp>
#include <drogular/template_parser.hpp>
#include <drogular/template_tokenizer.hpp>
#include <drogular/template_engine.hpp>

namespace drogular::template_compiler {

CompiledTemplate::CompiledTemplate(std::vector<NodePtr> nodes)
    : nodes_(std::move(nodes)) {
}

std::string CompiledTemplate::render(RenderContext& context) const {
    std::string html;

    for (const auto& node : nodes_) {
        if (node->type() == NodeType::Text) {
            const auto textNode =
                std::dynamic_pointer_cast<TextNode>(node);

            html += textNode->text();
        } else if (node->type() == NodeType::Variable) {
            const auto variableNode =
                std::dynamic_pointer_cast<VariableNode>(node);

            html += "{{ " + variableNode->expression() + " }}";
        } else if (node->type() == NodeType::RawVariable) {
            const auto rawNode =
                std::dynamic_pointer_cast<RawVariableNode>(node);

            html += "{{{ " + rawNode->expression() + " }}}";
        } else if (node->type() == NodeType::Component) {
            const auto componentNode =
                std::dynamic_pointer_cast<ComponentNode>(node);

            html += componentNode->tagHtml();
        }
    }

    return template_engine::render(html, context);
}

CompiledTemplate compile(std::string_view html) {
    return CompiledTemplate(
        parse(tokenize(html))
    );
}

} // namespace drogular::template_compiler
