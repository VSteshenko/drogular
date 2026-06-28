#include <drogular/page_support.hpp>
#include <drogular/render_context.hpp>
#include <drogular/pwa_page_support.hpp>

namespace drogular {

void PageSupport::apply(
    RenderContext& context,
    const std::string& pageTitle
) {
    PwaPageSupport::apply(context);

    context.set(
        "pageTitle",
        pageTitle
    );

    const auto request =
        context.request();

    context.set(
        "currentPath",
        request != nullptr
            ? request->path()
            : std::string("/")
    );
}

} // namespace drogular