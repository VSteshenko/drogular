#pragma once

#include <drogular/action_context.hpp>
#include <drogular/action_result.hpp>
#include <drogular/component.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/detail/request_context_bridge.hpp>
#include <drogular/render_context.hpp>

#include <drogon/HttpTypes.h>

#include <concepts>
#include <utility>

namespace drogular {

/**
 * Renders a component as an HTML action result while preserving the
 * request-bound state owned by the ActionContext.
 */
class ActionRenderer final {
public:
    template <typename ComponentType, typename Setup>
        requires std::derived_from<ComponentType, Component> &&
                 std::invocable<Setup, RenderContext&>
    static ActionResult render(
        ActionContext& actionContext,
        Setup&& setup,
        drogon::HttpStatusCode status = drogon::k200OK
    ) {
        auto renderContext =
            detail::RequestContextBridge::renderContext(
                actionContext
            );

        std::invoke(
            std::forward<Setup>(setup),
            renderContext
        );

        ComponentType component;

        return ActionResult::html(
            component_renderer::renderComponentTree(
                component,
                renderContext
            ),
            status
        );
    }

    template <typename ComponentType>
        requires std::derived_from<ComponentType, Component>
    static ActionResult render(
        ActionContext& actionContext,
        drogon::HttpStatusCode status = drogon::k200OK
    ) {
        return render<ComponentType>(
            actionContext,
            [](RenderContext&) {},
            status
        );
    }
};

} // namespace drogular