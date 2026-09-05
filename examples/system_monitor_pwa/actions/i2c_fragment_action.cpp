#include "i2c_fragment_action.hpp"
#include "ui/i2c_fragment_component.hpp"

#include <drogular/render_context.hpp>

#include <utility>

namespace system_monitor {

drogular::ActionResult I2cFragmentAction::handle(drogular::ActionContext& context) {
    drogular::RenderContext rc;
    rc.setServices(context.services());
    rc.setRequest(context.request());

    I2cFragmentComponent component;
    component.onInit(rc);

    auto html = component.render(rc);
    component.onDestroy(rc);

    return drogular::ActionResult::html(std::move(html));
}

} // namespace system_monitor