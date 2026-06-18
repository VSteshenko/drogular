#pragma once

#include "auth_store.hpp"

#include <drogular/page.hpp>

class AdminPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override
    {
        auto authStore =
            context.requireService<AuthStore>();

        const auto currentUser =
            authStore->currentUser.value();

        bool isAdmin = false;

        if (currentUser.has_value()) {
            isAdmin =
                currentUser->role == "admin";
        }

        context.set(
            "isAdmin",
            isAdmin
        );

        context.set(
            "username",
            currentUser.has_value()
                ? currentUser->username
                : std::string("")
        );
    }

    std::string templateHtml() const override
    {
        return R"(
@if(isAdmin)

<h1>Admin Panel</h1>

<p>
Welcome administrator {{ username }}.
</p>

@else

<h1>Access Denied</h1>

<p>
Administrator role required.
</p>

@endif
)";
    }
};