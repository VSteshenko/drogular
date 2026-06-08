#include <drogular/page.hpp>

namespace drogular {

std::optional<gql::Query> Page::query() const {
    return std::nullopt;
}

std::string TemplatePage::render(RenderContext& context) {
    return template_engine::render(
        templateHtml(),
        context
    );
}

} // namespace drogular
