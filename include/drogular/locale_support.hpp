#pragma once

#include <drogular/render_context.hpp>

#include <string>

namespace drogular {

/**
 * Provides helpers for resolving the current locale.
 *
 * LocaleSupport only determines the active locale.
 * It does not perform translation.
 */
class LocaleSupport {
public:
    /**
     * Returns the current locale or "en" when no locale is set.
     */
    static std::string current(
        RenderContext& context
    );

    /**
     * Returns the current locale or the provided fallback locale
     * when no locale is set.
     */
    static std::string current(
        RenderContext& context,
        const std::string& defaultLocale
    );
};

} // namespace drogular