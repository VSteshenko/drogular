#pragma once

#include "user_repository.hpp"

#include <drogular/page.hpp>

class RepositorySampleUsersPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override
    {
        auto repository =
            context.requireService<
                RepositorySampleUserRepository
            >();

        Json::Value users(Json::arrayValue);

        for (const auto& sourceUser :
             repository->all()) {
            Json::Value user;

            user["id"] = sourceUser.id;
            user["name"] = sourceUser.name;
            user["email"] = sourceUser.email;

            users.append(user);
        }

        context.set(
            "pageTitle",
            std::string("Users")
        );

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