#include "system_fragment_action.hpp"
#include "ui/system_fragment_component.hpp"

#include <drogular/action_renderer.hpp>

namespace system_monitor {

drogular::ActionResult SystemFragmentAction::handle(
    drogular::ActionContext& context) {
    return drogular::ActionRenderer::render<
        SystemFragmentComponent
    >(context);
}

} // namespace system_monitor