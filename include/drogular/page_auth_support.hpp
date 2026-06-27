#pragma once

#include <drogular/component.hpp>

#include <string>

namespace drogular {

/**
 * Provides page-level authentication helpers.
 *
 * PageAuthSupport is intended for TemplatePage/Page code.
 * It sets common render context flags and returns whether
 * rendering should continue.
 */
class PageAuthSupport {
public:
    /**
     * Requires an authenticated session.
     *
     * Sets loginRequired to true when no valid session exists.
     * Returns true when the page can continue rendering.
     */
    static bool requireAuthentication(
        RenderContext& context
    );

    /**
     * Requires a specific session value.
     *
     * Sets accessDenied to true when the session value is missing
     * or does not match the expected value.
     */
    static bool requireSessionValue(
        RenderContext& context,
        const std::string& key,
        const std::string& expectedValue
    );
};

} // namespace drogular