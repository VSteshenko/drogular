#include "gpio_fragment_action.hpp"
#include "ui/gpio_fragment_component.hpp"

#include <drogular/action_renderer.hpp>

namespace system_monitor {

drogular::ActionResult GpioFragmentAction::handle(drogular::ActionContext& context) {
    return drogular::ActionRenderer::render<
        GpioFragmentComponent
    >(context);
}

} // namespace system_monitor