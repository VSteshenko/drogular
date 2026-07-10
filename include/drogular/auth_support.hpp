#pragma once

#include <drogular/render_context.hpp>
#include <drogular/action_context.hpp>

#include <optional>
#include <string>

namespace drogular {

/**
 * Provides small helpers for reading authentication-related
 * values from the current session.
 *
 * AuthSupport does not define users, roles or permissions.
 * It only provides generic session-based helpers that
 * applications can build on top of.
 */
class AuthSupport {
public:
    /**
     * Returns true when the current request has a valid session.
     */
    static bool isAuthenticated(
        RenderContext& context
    );

    /**
     * Returns true when the current request has a valid session.
     */
    static bool isAuthenticated(
        ActionContext& context
    );

    /**
     * Returns a string value from the current session.
     */
    static std::optional<std::string> sessionValue(
        RenderContext& context,
        const std::string& key
    );

    /**
     * Returns a string value from the current session.
     */
    static std::optional<std::string> sessionValue(
        ActionContext& context,
        const std::string& key
    );

    /**
     * Returns true when a session value exists and matches
     * the expected value.
     */
    static bool hasSessionValue(
        RenderContext& context,
        const std::string& key,
        const std::string& expectedValue
    );

    /**
     * Returns true when a session value exists and matches
     * the expected value.
     */
    static bool hasSessionValue(
        ActionContext& context,
        const std::string& key,
        const std::string& expectedValue
    );
};

} // namespace drogular