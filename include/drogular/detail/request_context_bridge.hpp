#pragma once

#include <drogular/action_context.hpp>
#include <drogular/render_context.hpp>

namespace drogular::detail {

/**
 * Internal bridge between request-bound framework contexts.
 *
 * This preserves the RequestContextState when rendering from an action.
 * Application code should use the higher-level action rendering API.
 */
class RequestContextBridge final {
public:
    // Internal integration point: callers must authenticate before binding.
    static void bindSecurityContext(
        ActionContext& actionContext,
        security::SecurityContext context
    ) {
        actionContext.state_->setSecurityContext(std::move(context));
    }

    static RenderContext renderContext(
        const ActionContext& actionContext
    ) {
        return RenderContext(actionContext.state_);
    }
};

} // namespace drogular::detail