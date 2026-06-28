#pragma once

#include <string>

namespace drogular {

/**
 * Provides URL utility helpers.
 */
class Url {
public:
    /**
     * Percent-encodes a value for safe use in URL query strings.
     */
    static std::string encode(
        const std::string& value
    );
};

} // namespace drogular