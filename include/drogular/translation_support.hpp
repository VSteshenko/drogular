#pragma once

#include <drogular/render_context.hpp>

#include <string>

namespace drogular {

/**
 * Provides helpers for translating keys using the current locale.
 */
class TranslationSupport {
public:
    /**
     * Translates the key using the current locale and the registered
     * TranslationProvider service.
     *
     * If no TranslationProvider is registered, the key itself is returned.
     */
    static std::string translate(
        RenderContext& context,
        const std::string& key
    );

    /**
     * Translates the key using an explicit locale.
     *
     * If no TranslationProvider is registered, the key itself is returned.
     */
    static std::string translate(
        RenderContext& context,
        const std::string& locale,
        const std::string& key
    );
};

} // namespace drogular