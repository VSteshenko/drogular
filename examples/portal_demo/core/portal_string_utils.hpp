#pragma once

#include <algorithm>
#include <cctype>
#include <string>

inline std::string portalAsciiLowercase(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        }
    );

    return value;
}