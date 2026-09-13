#include "offline_page.hpp"
#include "localization/system_monitor_translations.hpp"

#include <drogular/pwa_page_support.hpp>
#include <drogular/render_context.hpp>

#include <string>

namespace system_monitor {

void OfflinePage::onInit(drogular::RenderContext& context) {
    applyLocalization(context);
    context.set("languageRedirect", std::string("/"));
    drogular::PwaOptions pwaOptions;
    pwaOptions.themeColor = "#10172a";
    drogular::PwaPageSupport::apply(context, pwaOptions);

    context.set("title", context.translate("app.title"));
    context.set("hasPageScript", false);
}

} // namespace system_monitor