#pragma once

#include <drogular/page.hpp>
#include <drogular/pwa_page_support.hpp>

class TodoPwaOfflinePage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override
    {
        drogular::PwaPageSupport::apply(context);

        context.set(
            "pageTitle",
            std::string("Offline")
        );
    }

    std::string templatePath() const override
    {
        return "offline.html";
    }

    std::string layoutPath() const override
    {
        return "layouts/main.html";
    }
};