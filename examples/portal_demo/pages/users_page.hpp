#pragma once

#include "../auth/portal_auth_support.hpp"
#include "../ui/portal_page_support.hpp"
#include "../portal_user_repository.hpp"
#include "../localization/portal_error_translator.hpp"
#include "../localization/portal_locale.hpp"

#include <drogular/page.hpp>

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

        const auto locale =
            PortalLocale::fromRenderContext(context);

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

        auto translations =
            context.requireService<PortalTranslations>();

        const auto usersError =
            PortalErrorTranslator::usersError(
                *translations,
                locale,
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
                *translations,
                locale,
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

        const auto currentUser =
            PortalAuthSupport::currentUser(context);

        context.set(
            "loginRequired",
            !currentUser.has_value()
        );

        if (!currentUser.has_value()) {
            return;
        }

        auto repository =
            context.requireService<PortalUserRepository>();

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