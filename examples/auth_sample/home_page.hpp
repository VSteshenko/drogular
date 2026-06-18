#pragma once

#include "auth_store.hpp"

#include <drogular/page.hpp>

class HomePage final
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

        context.set(
            "authenticated",
            currentUser.has_value()
        );
    }

    std::string templateHtml() const override
    {
        return R"(
@if(authenticated)
<meta http-equiv="refresh" content="0; url=/dashboard" />
<p>Redirecting to dashboard...</p>
@else
<meta http-equiv="refresh" content="0; url=/login" />
<p>Redirecting to login...</p>
@endif
)";
    }
};