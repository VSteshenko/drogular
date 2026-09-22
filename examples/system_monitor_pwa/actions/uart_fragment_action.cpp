#include "uart_fragment_action.hpp"
#include "ui/uart_fragment_component.hpp"

#include <drogular/action_renderer.hpp>

namespace system_monitor {

drogular::ActionResult UartFragmentAction::handle(drogular::ActionContext& context) {
    return drogular::ActionRenderer::render<
        UartFragmentComponent
    >(context);
}

} // namespace system_monitor