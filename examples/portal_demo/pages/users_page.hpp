#pragma once

#include "../auth/portal_auth_support.hpp"
#include "../ui/portal_page_support.hpp"
#include "../providers/user_provider.hpp"
#include "../providers/role_provider.hpp"
#include "../localization/portal_error_translator.hpp"
#include "../data/portal_schema.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

class PortalUsersPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "users.title"
        );

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

        const auto usersError =
            PortalErrorTranslator::usersError(
                context,
                error
            );

        const auto username =
            request != nullptr
                ? request->getParameter("username")
                : std::string("");

        context.set(
            "createUsername",
            username
        );

        const auto usersSuccess =
            PortalErrorTranslator::usersSuccess(
                context,
                success
            );

        context.set(
            "hasUsersError",
            !usersError.empty()
        );

        context.set(
            "usersError",
            usersError
        );

        context.set(
            "hasUsersSuccess",
            !usersSuccess.empty()
        );

        context.set(
            "usersSuccess",
            usersSuccess
        );

        context.set(
            "alertMessage",
            !usersError.empty()
                ? usersError
                : usersSuccess
        );

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        const auto schema =
            PortalSchema::users();

        context.set(
            "usersUsername",
            context.translate(schema.fieldLabelKey("username"))
        );
        context.set(
            "usersPassword",
            context.translate(schema.fieldLabelKey("password"))
        );
        context.set(
            "usersRole",
            context.translate(schema.fieldLabelKey("role"))
        );

        context.set(
            "usersRoleRequired",
            schema.fieldRequired("role")
        );

        auto repository =
            context.requireService<PortalUserProvider>();

        auto roles =
            context.requireService<PortalRoleProvider>();

        Json::Value roleOptions(Json::arrayValue);

        for (const auto& role : roles->all()) {
            Json::Value option(Json::objectValue);

            option["value"] = role.code;
            option["label"] = role.title;
            option["selected"] = false;

            roleOptions.append(
                std::move(option)
            );
        }

        context.set("roleOptions", roleOptions);

        Json::Value users(Json::arrayValue);

        for (const auto& user : repository->all()) {
            Json::Value value;
            value["username"] = user.username;
            value["role"] = user.role;
            users.append(value);
        }

        context.set("users", users);
    }

    std::string templatePath() const override {
        return "users.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};