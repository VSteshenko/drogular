#include "i2c_fragment_action.hpp"
#include "ui/i2c_fragment_component.hpp"

#include <drogular/action_renderer.hpp>

namespace system_monitor {

drogular::ActionResult I2cFragmentAction::handle(drogular::ActionContext& context) {
    return drogular::ActionRenderer::render<
        I2cFragmentComponent
    >(context);
}

} // namespace system_monitor