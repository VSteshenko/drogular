#pragma once

#include "../portal_page_support.hpp"

#include <drogular/page.hpp>

class PortalLoginPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override
    {
        PortalPageSupport::apply(
            context,
            "login.title"
        );
    }

    std::string templatePath() const override
    {
        return "login.html";
    }

    std::string layoutPath() const override
    {
        return "layouts/main.html";
    }
};