#include "offline_page.hpp"

#include <drogular/pwa_page_support.hpp>
#include <drogular/render_context.hpp>

#include <string>

namespace system_monitor {

void OfflinePage::onInit(drogular::RenderContext& context) {
    drogular::PwaOptions pwaOptions;
    pwaOptions.themeColor = "#10172a";
    drogular::PwaPageSupport::apply(context, pwaOptions);

    context.set("title", std::string("Drogular System Monitor"));
    context.set("hasPageScript", false);
}

} // namespace system_monitor