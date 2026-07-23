#include <drogular/page.hpp>
#include <drogular/render_context.hpp>

namespace drogular {

std::optional<gql::Query> Page::query() const {
    return std::nullopt;
}

std::string TemplatePage::render(RenderContext& context) {
    return renderTemplate(context);
}

} // namespace drogular