#include "hardware_fragment_action.hpp"
#include "ui/hardware_fragment_component.hpp"

#include <drogular/action_renderer.hpp>

namespace system_monitor {

drogular::ActionResult HardwareFragmentAction::handle(drogular::ActionContext& context) {
    return drogular::ActionRenderer::render<
        HardwareFragmentComponent
    >(context);
}

} // namespace system_monitor