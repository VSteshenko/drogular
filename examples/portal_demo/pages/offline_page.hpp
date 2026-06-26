#pragma once

#include "../ui/portal_page_support.hpp"

#include <drogular/page.hpp>

class PortalOfflinePage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "offline.title"
        );
    }

    std::string templatePath() const override {
        return "offline.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};