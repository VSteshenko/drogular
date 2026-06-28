#pragma once

#include <drogular/action_context.hpp>
#include <drogular/action_result.hpp>

#include <optional>
#include <string>

namespace drogular {

/**
 * Provides action-level authentication helpers.
 */
class ActionAuthSupport {
public:
    /**
     * Requires an authenticated action session.
     *
     * Returns redirect result when no active session exists.
     * Returns std::nullopt when the action may continue.
     */
    static std::optional<ActionResult> requireAuthentication(
        ActionContext& context,
        const std::string& redirectPath = "/login"
    );

    /**
     * Requires a specific session value.
     *
     * Returns redirect result when the session value is missing
     * or does not match the expected value.
     */
    static std::optional<ActionResult> requireSessionValue(
        ActionContext& context,
        const std::string& key,
        const std::string& expectedValue,
        const std::string& redirectPath
    );
};

} // namespace drogular