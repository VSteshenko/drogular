#pragma once

#include <drogular/action_context.hpp>
#include <drogular/action_result.hpp>

namespace drogular {

/**
 * Base class for application action handlers.
 */
class ActionHandler {
public:
    virtual ~ActionHandler() = default;

    /**
     * Executes an application action.
     */
    virtual ActionResult handle(ActionContext& context) = 0;
};

} // namespace drogular
