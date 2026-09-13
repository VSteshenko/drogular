#include "dashboard_page.hpp"

#include "localization/system_monitor_translations.hpp"
#include "ui/system_fragment_component.hpp"

#include <drogular/pwa_page_support.hpp>
#include <drogular/render_context.hpp>

#include <string>
#include <utility>

namespace system_monitor {

void DashboardPage::onInit(drogular::RenderContext& context) {
    applyLocalization(context);
    context.set("languageRedirect", std::string("/"));

    drogular::PwaOptions pwaOptions;
    pwaOptions.themeColor = "#10172a";
    drogular::PwaPageSupport::apply(context, pwaOptions);

    context.set("title", context.translate("app.title"));
    context.set("hasPageScript", true);

    SystemFragmentComponent component;
    component.onInit(context);
    auto html = component.render(context);
    component.onDestroy(context);
    context.set("systemFragmentHtml", std::move(html));
}

} // namespace system_monitor