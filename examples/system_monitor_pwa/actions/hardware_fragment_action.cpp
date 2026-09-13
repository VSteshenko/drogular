#include "hardware_fragment_action.hpp"
#include "ui/hardware_fragment_component.hpp"

#include <drogular/render_context.hpp>

#include <utility>

namespace system_monitor {

drogular::ActionResult HardwareFragmentAction::handle(drogular::ActionContext& context) {
    drogular::RenderContext rc;
    rc.setServices(context.services());
    rc.setRequest(context.request());

    HardwareFragmentComponent component;
    component.onInit(rc);
    auto html = component.render(rc);
    component.onDestroy(rc);

    return drogular::ActionResult::html(std::move(html));
}

} // namespace system_monitor