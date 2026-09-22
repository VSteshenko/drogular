#include "process_fragment_action.hpp"
#include "ui/processes_fragment_component.hpp"

#include <drogular/action_renderer.hpp>

namespace system_monitor {

drogular::ActionResult ProcessFragmentAction::handle(drogular::ActionContext& context) {
    return drogular::ActionRenderer::render<
        ProcessesFragmentComponent
    >(context);
}

} // namespace system_monitor