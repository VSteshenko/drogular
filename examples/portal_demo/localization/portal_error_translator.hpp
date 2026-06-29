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
            return context.translate("login.error.missing_credentials");
        }

        if (error == "invalid_credentials") {
            return context.translate("login.error.invalid_credentials");
        }

        return "";
    }

    static std::string usersError(
        drogular::RenderContext& context,
        const std::string& error
    ) {
        if (error == "validation") {
            return context.translate("users.error.validation");
        }

        if (error == "access_denied") {
            return context.translate("users.error.access_denied");
        }

        if (error == "duplicate_user") {
            return context.translate("users.error.duplicate_user");
        }

        return "";
    }

    static std::string usersSuccess(
        drogular::RenderContext& context,
        const std::string& success
    ) {
        if (success == "user_created") {
            return context.translate("users.success.created");
        }

        return "";
    }

    static std::string projectsError(
        drogular::RenderContext& context,
        const std::string& error
    ) {
        if (error == "validation") {
            return context.translate("projects.error.validation");
        }
        if (error == "not_found") {
            return context.translate("projects.error.not_found");
        }
        if (error == "not_found") {
            return context.translate("projects.error.not_found");
        }

        return "";
    }

    static std::string projectsSuccess(
        drogular::RenderContext& context,
        const std::string& success
    ) {
        if (success == "project_created") {
            return context.translate("projects.success.created");
        }
        if (success == "project_updated") {
            return context.translate("projects.success.updated");
        }
        if (success == "project_deleted") {
            return context.translate("projects.success.deleted");
        }

        return "";
    }
};