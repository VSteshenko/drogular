#pragma once

#include "../data/portal_schema.hpp"
#include "../localization/portal_error_translator.hpp"
#include "../providers/role_provider.hpp"
#include "../providers/user_provider.hpp"
#include "../ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <cstdlib>
#include <string>

class PortalUserEditPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "users.edit.title"
        );

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
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

        auto users =
            context.requireService<PortalUserProvider>();

        const auto user =
            users->findById(id);

        context.set(
            "userNotFound",
            !user.has_value()
        );

        if (!user.has_value()) {
            return;
        }

        const auto request =
            context.request();

        const auto error =
            request != nullptr
                ? request->getParameter("error")
                : std::string("");

        const auto usersError =
            PortalErrorTranslator::usersError(
                context,
                error
            );

        const auto schema =
            PortalSchema::users();

        context.set(
            "usernameLabel",
            context.translate(
                schema.fieldLabelKey("username")
            )
        );

        context.set(
            "roleLabel",
            context.translate(
                schema.fieldLabelKey("role")
            )
        );

        context.set(
            "usernameRequired",
            schema.fieldRequired("username")
        );

        context.set(
            "roleRequired",
            schema.fieldRequired("role")
        );

        auto roles =
            context.requireService<PortalRoleProvider>();

        Json::Value roleOptions(
            Json::arrayValue
        );

        for (const auto& role : roles->all()) {
            Json::Value option(
                Json::objectValue
            );

            option["value"] =
                role.code;

            option["label"] =
                role.title;

            option["selected"] =
                role.code == user->role;

            roleOptions.append(
                std::move(option)
            );
        }

        context.set(
            "roleOptions",
            roleOptions
        );

        context.set(
            "hasUsersError",
            !usersError.empty()
        );

        context.set(
            "alertMessage",
            usersError
        );

        context.set(
            "userId",
            user->id
        );

        context.set(
            "username",
            user->username
        );
    }

    std::string templatePath() const override {
        return "user_edit.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};