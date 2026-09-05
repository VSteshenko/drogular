#include "spi_fragment_action.hpp"
#include "ui/spi_fragment_component.hpp"

#include <drogular/render_context.hpp>

#include <utility>

namespace system_monitor {

drogular::ActionResult SpiFragmentAction::handle(drogular::ActionContext& context) {
    drogular::RenderContext rc;
    rc.setServices(context.services());
    rc.setRequest(context.request());

    SpiFragmentComponent component;
    component.onInit(rc);

    auto html=component.render(rc);
    component.onDestroy(rc);

    return drogular::ActionResult::html(std::move(html));
}

} // namespace system_monitor