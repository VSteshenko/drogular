#include "system_fragment_action.hpp"
#include "ui/system_fragment_component.hpp"

#include <drogular/render_context.hpp>

#include <utility>

namespace system_monitor {

drogular::ActionResult SystemFragmentAction::handle(
    drogular::ActionContext& context) {
    drogular::RenderContext renderContext;
    renderContext.setServices(context.services());
    renderContext.setRequest(context.request());

    SystemFragmentComponent component;
    component.onInit(renderContext);
    auto html = component.render(renderContext);
    component.onDestroy(renderContext);

    return drogular::ActionResult::html(std::move(html));
}

} // namespace system_monitor