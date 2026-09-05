#include "uart_fragment_action.hpp"
#include "ui/uart_fragment_component.hpp"

#include <drogular/render_context.hpp>

#include <utility>

namespace system_monitor {

drogular::ActionResult UartFragmentAction::handle(drogular::ActionContext& context) {
    drogular::RenderContext rc;
    rc.setServices(context.services());
    rc.setRequest(context.request());

    UartFragmentComponent component;
    component.onInit(rc);

    auto html=component.render(rc);
    component.onDestroy(rc);

    return drogular::ActionResult::html(std::move(html));
}

} // namespace system_monitor