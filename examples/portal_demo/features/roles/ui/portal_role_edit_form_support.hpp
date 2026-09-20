#pragma once

#include "data/portal_schema.hpp"
#include "features/localization/support/portal_error_translator.hpp"

#include <drogular/render_context.hpp>

#include <string>

class PortalRoleEditFormSupport final {
public:
    static void apply(
        drogular::RenderContext& context,
        int roleId,
        const std::string& code,
        const std::string& title,
        const std::string& error = {},
        const std::string& success = {}
    ) {
        const auto rolesError =
            PortalErrorTranslator::rolesError(context, error);
        const auto rolesSuccess =
            PortalErrorTranslator::rolesSuccess(context, success);

        context.set("hasRolesError", !rolesError.empty());
        context.set("hasRolesSuccess", !rolesSuccess.empty());
        context.set(
            "alertMessage",
            !rolesError.empty() ? rolesError : rolesSuccess
        );

        const auto schema = PortalSchema::roles();

        context.set("roleId", roleId);
        context.set("roleCode", code);
        context.set("roleTitle", title);
        context.set(
            "roleCodeLabel",
            context.translate(schema.fieldLabelKey("code"))
        );
        context.set(
            "roleTitleLabel",
            context.translate(schema.fieldLabelKey("title"))
        );
        context.set(
            "roleCodeRequired",
            schema.fieldRequired("code")
        );
        context.set(
            "roleTitleRequired",
            schema.fieldRequired("title")
        );
    }
};