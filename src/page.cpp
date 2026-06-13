#include <drogular/page.hpp>
#include <drogular/component_renderer.hpp>

namespace drogular {

std::optional<gql::Query> Page::query() const {
    return std::nullopt;
}

std::string TemplatePage::render(RenderContext& context) {
    const auto compiled =
        templateCache_.getOrCompile(templateHtml());

    auto html = compiled->render(context);

    if (context.services() != nullptr) {
        html = component_renderer::render(
            html,
            context.services()->components(),
            context
        );
    }

    return html;
}

} // namespace drogular