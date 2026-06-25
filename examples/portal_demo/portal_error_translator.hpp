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

    static std::string usersError(
        const std::string& locale,
        const std::string& error
    ) {
        PortalTranslations translations;

        if (error == "validation") {
            return translations.get(
                locale,
                "users.error.validation"
            );
        }

        if (error == "access_denied") {
            return translations.get(
                locale,
                "users.error.access_denied"
            );
        }

        return "";
    }

    static std::string usersSuccess(
        const std::string& locale,
        const std::string& success
    ) {
        PortalTranslations translations;

        if (success == "user_created") {
            return translations.get(
                locale,
                "users.success.created"
            );
        }

        return "";
    }
};