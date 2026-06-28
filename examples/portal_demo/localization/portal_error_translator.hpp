#pragma once

#include <drogular/translation_support.hpp>

#include <string>

class PortalErrorTranslator {
public:
    static std::string loginError(
        drogular::RenderContext& context,
        const std::string& error
    ) {
        if (error == "missing_credentials") {
            return drogular::TranslationSupport::translate(
                context,
                "login.error.missing_credentials"
            );
        }

        if (error == "invalid_credentials") {
            return drogular::TranslationSupport::translate(
                context,
                "login.error.invalid_credentials"
            );
        }

        return "";
    }

    static std::string usersError(
        drogular::RenderContext& context,
        const std::string& error
    ) {
        if (error == "validation") {
            return drogular::TranslationSupport::translate(
                context,
                "users.error.validation"
            );
        }

        if (error == "access_denied") {
            return drogular::TranslationSupport::translate(
                context,
                "users.error.access_denied"
            );
        }

        if (error == "duplicate_user") {
            return drogular::TranslationSupport::translate(
                context,
                "users.error.duplicate_user"
            );
        }

        return "";
    }

    static std::string usersSuccess(
        drogular::RenderContext& context,
        const std::string& success
    ) {
        if (success == "user_created") {
            return drogular::TranslationSupport::translate(
                context,
                "users.success.created"
            );
        }

        return "";
    }

    static std::string projectsError(
        drogular::RenderContext& context,
        const std::string& error
    ) {
        if (error == "validation") {
            return drogular::TranslationSupport::translate(
                context,
                "projects.error.validation"
            );
        }

        return "";
    }

    static std::string projectsSuccess(
        drogular::RenderContext& context,
        const std::string& success
    ) {
        if (success == "project_created") {
            return drogular::TranslationSupport::translate(
                context,
                "projects.success.created"
            );
        }

        return "";
    }
};