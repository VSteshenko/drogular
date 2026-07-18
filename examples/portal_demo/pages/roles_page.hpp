#pragma once

#include "data/portal_schema.hpp"
#include "localization/portal_error_translator.hpp"
#include "providers/role_provider.hpp"
#include "features/users/providers/user_provider.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <string>
#include <unordered_map>

class PortalRolesPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "roles.page.title"
        );

        context.set("hasAdminAccess", false);

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

        context.set("hasAdminAccess", true);

        const auto request =
            context.request();

        const auto error =
            request != nullptr
                ? request->getParameter("error")
                : std::string("");

        const auto success =
            request != nullptr
                ? request->getParameter("success")
                : std::string("");

        const auto code =
            request != nullptr
                ? request->getParameter("code")
                : std::string("");

        const auto title =
            request != nullptr
                ? request->getParameter("title")
                : std::string("");

        const auto rolesError =
            PortalErrorTranslator::rolesError(
                context,
                error
            );

        const auto rolesSuccess =
            PortalErrorTranslator::rolesSuccess(
                context,
                success
            );

        context.set("hasRolesError", !rolesError.empty());
        context.set("hasRolesSuccess", !rolesSuccess.empty());
        context.set(
            "alertMessage",
            !rolesError.empty()
                ? rolesError
                : rolesSuccess
        );

        context.set(
            "createRoleCode",
            code
        );

        context.set(
            "createRoleTitle",
            title
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

        auto roles =
            context.requireService<PortalRoleProvider>();

        auto users =
            context.requireService<PortalUserProvider>();

        std::unordered_map<std::string, int>
            userCounts;

        for (const auto& user : users->all()) {
            ++userCounts[user.role];
        }

        Json::Value items(
            Json::arrayValue
        );

        for (const auto& role : roles->all()) {
            const auto found =
                userCounts.find(role.code);

            const auto userCount =
                found != userCounts.end()
                    ? found->second
                    : 0;

            Json::Value item(Json::objectValue);

            item["title"] = role.title;
            item["subtitle"] = role.code;
            item["count"] = userCount;

            item["countLabel"] =
                context.translate("roles.users");

            item["canDelete"] =
                userCount == 0;

            item["editUrl"] =
                "/roles/" +
                std::to_string(role.id) +
                "/edit";

            item["deleteUrl"] =
                "/roles/" +
                std::to_string(role.id) +
                "/delete";

            items.append(
                std::move(item)
            );
        }

        context.set(
            "roles",
            items
        );
    }

    std::string templatePath() const override {
        return "roles.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};