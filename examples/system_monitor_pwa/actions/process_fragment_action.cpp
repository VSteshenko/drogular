#include "process_fragment_action.hpp"
#include "ui/processes_fragment_component.hpp"

#include <drogular/render_context.hpp>

#include <utility>

namespace system_monitor {

drogular::ActionResult ProcessFragmentAction::handle(drogular::ActionContext& context) {
    drogular::RenderContext renderContext;
    renderContext.setServices(context.services());
    renderContext.setRequest(context.request());

    ProcessesFragmentComponent component;
    component.onInit(renderContext);
    auto html = component.render(renderContext);
    component.onDestroy(renderContext);
    return drogular::ActionResult::html(std::move(html));
}

} // namespace system_monitor