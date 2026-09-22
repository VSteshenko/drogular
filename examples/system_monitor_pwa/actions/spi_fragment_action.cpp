#include "spi_fragment_action.hpp"
#include "ui/spi_fragment_component.hpp"

#include <drogular/action_renderer.hpp>

namespace system_monitor {

drogular::ActionResult SpiFragmentAction::handle(drogular::ActionContext& context) {
    return drogular::ActionRenderer::render<
        SpiFragmentComponent
    >(context);
}

} // namespace system_monitor