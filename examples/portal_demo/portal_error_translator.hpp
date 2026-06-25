#pragma once

#include "portal_translations.hpp"

#include <string>

class PortalErrorTranslator {
public:
    static std::string loginError(
        const std::string& locale,
        const std::string& error
    ) {
        PortalTranslations translations;

        if (error == "missing_credentials") {
            return translations.get(
                locale,
                "login.error.missing_credentials"
            );
        }

        if (error == "invalid_credentials") {
            return translations.get(
                locale,
                "login.error.invalid_credentials"
            );
        }

        return "";
    }
};