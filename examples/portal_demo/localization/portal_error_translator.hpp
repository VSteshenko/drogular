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

        if (success == "user_updated") {
            return context.translate("users.success.updated");
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

    static std::string projectTypesError(
        drogular::RenderContext& context,
        const std::string& error
    ) {
        if (error == "validation") {
            return context.translate(
                "project_types.error.validation"
            );
        }

        if (error == "duplicate_code") {
            return context.translate(
                "project_types.error.duplicate_code"
            );
        }

        if (error == "not_found") {
            return context.translate(
                "project_types.error.not_found"
            );
        }

        if (error == "project_type_in_use") {
            return context.translate(
                "project_types.error.in_use"
            );
        }

        return "";
    }

    static std::string projectTypesSuccess(
        drogular::RenderContext& context,
        const std::string& success
    ) {
        if (success == "project_type_created") {
            return context.translate(
                "project_types.success.created"
            );
        }

        if (success == "project_type_updated") {
            return context.translate(
                "project_types.success.updated"
            );
        }

        if (success == "project_type_deleted") {
            return context.translate(
                "project_types.success.deleted"
            );
        }

        return "";
    }

    static std::string rolesError(
        drogular::RenderContext& context,
        const std::string& error
    ) {
        if (error == "validation") {
            return context.translate("roles.error.validation");
        }

        if (error == "duplicate_code") {
            return context.translate("roles.error.duplicate_code");
        }

        if (error == "access_denied") {
            return context.translate("roles.error.access_denied");
        }

        if (error == "not_found") {
            return context.translate(
                "roles.error.not_found"
            );
        }

        if (error == "role_in_use") {
            return context.translate(
                "roles.error.in_use"
            );
        }

        return "";
    }

    static std::string rolesSuccess(
        drogular::RenderContext& context,
        const std::string& success
    ) {
        if (success == "role_created") {
            return context.translate("roles.success.created");
        }

        if (success == "role_updated") {
            return context.translate(
                "roles.success.updated"
            );
        }

        if (success == "role_deleted") {
            return context.translate(
                "roles.success.deleted"
            );
        }

        return "";
    }
};