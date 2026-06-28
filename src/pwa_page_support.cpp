#include <drogular/pwa_page_support.hpp>
#include <drogular/render_context.hpp>
#include <drogular/pwa_html.hpp>
#include <drogular/pwa_scripts.hpp>

namespace drogular {

void PwaPageSupport::apply(
    RenderContext& context,
    const PwaOptions& options
) {
    context.set(
        "manifestLink",
        PwaHtml::manifestLink(
            options.manifestPath
        )
    );

    context.set(
        "faviconLink",
        PwaHtml::favicon(
            options.faviconPath,
            options.faviconType
        )
    );

    context.set(
        "themeColorMeta",
        PwaHtml::themeColor(
            options.themeColor
        )
    );

    context.set(
        "serviceWorkerRegistration",
        PwaScripts::serviceWorkerRegistration(
            options.serviceWorkerPath
        )
    );
}

} // namespace drogular
