#pragma once

#include "portal_translations.hpp"

#include <string>

class PortalErrorTranslator {
public:
    static std::string loginError(
        const PortalTranslations& translations,
        const std::string& locale,
        const std::string& error
    ) {
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
        const PortalTranslations& translations,
        const std::string& locale,
        const std::string& error
    ) {
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

        if (error == "duplicate_user") {
            return translations.get(
                locale,
                "users.error.duplicate_user"
            );
        }

        return "";
    }

    static std::string usersSuccess(
        const PortalTranslations& translations,
        const std::string& locale,
        const std::string& success
    ) {
        if (success == "user_created") {
            return translations.get(
                locale,
                "users.success.created"
            );
        }

        return "";
    }

    static std::string projectsError(
        const PortalTranslations& translations,
        const std::string& locale,
        const std::string& error
    ) {
        if (error == "validation") {
            return translations.get(
                locale,
                "projects.error.validation"
            );
        }

        return "";
    }

    static std::string projectsSuccess(
        const PortalTranslations& translations,
        const std::string& locale,
        const std::string& success
    ) {
        if (success == "project_created") {
            return translations.get(
                locale,
                "projects.success.created"
            );
        }

        return "";
    }
};