#pragma once

#include "data/portal_schema.hpp"
#include "localization/portal_error_translator.hpp"
#include "providers/role_provider.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <cstdlib>
#include <string>

class PortalRoleEditPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "roles.edit_page.title"
        );

        if (!drogular::PageAuthSupport::requireAuthentication(
                context
            )) {
            return;
        }

        if (!drogular::PageAuthSupport::requireSessionValue(
                context,
                "role",
                "admin"
            )) {
            return;
        }

        const auto id =
            std::atoi(
                context.requireRouteParam("id").c_str()
            );

        auto roles =
            context.requireService<PortalRoleProvider>();

        const auto role =
            roles->findById(id);

        context.set("roleNotFound", !role.has_value());

        if (!role.has_value()) {
            return;
        }

        const auto request =
            context.request();

        const auto error =
            request != nullptr
                ? request->getParameter("error")
                : std::string("");

        const auto rolesError =
            PortalErrorTranslator::rolesError(
                context,
                error
            );

        const auto schema =
            PortalSchema::roles();

        context.set(
            "roleCodeLabel",
            context.translate(
                schema.fieldLabelKey("code")
            )
        );

        context.set(
            "roleTitleLabel",
            context.translate(
                schema.fieldLabelKey("title")
            )
        );

        context.set(
            "roleCodeRequired",
            schema.fieldRequired("code")
        );

        context.set(
            "roleTitleRequired",
            schema.fieldRequired("title")
        );

        context.set(
            "hasRolesError",
            !rolesError.empty()
        );

        context.set(
            "alertMessage",
            rolesError
        );

        context.set(
            "roleId",
            role->id
        );

        context.set(
            "roleCode",
            role->code
        );

        context.set(
            "roleTitle",
            role->title
        );
    }

    std::string templatePath() const override {
        return "role_edit.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};