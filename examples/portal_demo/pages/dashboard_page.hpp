#pragma once

#include "../portal_page_support.hpp"

#include <drogular/page.hpp>

class PortalDashboardPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override
    {
        PortalPageSupport::apply(
            context,
            "dashboard.title"
        );
    }

    std::string templatePath() const override
    {
        return "dashboard.html";
    }

    std::string layoutPath() const override
    {
        return "layouts/main.html";
    }
};
