#pragma once

#include "../portal_auth_support.hpp"
#include "../portal_page_support.hpp"
#include "../portal_user_repository.hpp"

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

        const auto currentUser =
            PortalAuthSupport::currentUser(context);

        context.set(
            "loginRequired",
            !currentUser.has_value()
        );

        if (!currentUser.has_value()) {
            return;
        }

        PortalUserRepository repository;

        Json::Value users(Json::arrayValue);

        for (const auto& user : repository.all()) {
            Json::Value value;
            value["username"] = user.username;
            value["role"] = user.role;
            users.append(value);
        }

        context.set("users", users);
    }

    std::string templatePath() const override
    {
        return "users.html";
    }

    std::string layoutPath() const override
    {
        return "layouts/main.html";
    }
};