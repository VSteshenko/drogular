#pragma once

#include <string>

namespace drogular {

/**
 * Provides translated text for a locale and translation key.
 */
class TranslationProvider {
public:
    virtual ~TranslationProvider() = default;

    /**
     * Returns the translated value for the locale and key.
     */
    virtual std::string translate(
        const std::string& locale,
        const std::string& key
    ) const = 0;
};

} // namespace drogular