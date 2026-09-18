#pragma once

#include "features/projects/ui/portal_project_create_form_support.hpp"
#include "features/projects/ui/portal_projects_browser_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <algorithm>

class PortalProjectsPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "projects.title"
        );

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

        const auto createTitle =
            request != nullptr
                ? request->getParameter("title")
                : std::string("");

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        PortalProjectCreateFormSupport::apply(
            context,
            createTitle,
            {},
            "active",
            error,
            success
        );

        PortalProjectsBrowserSupport::apply(context);
    }

    std::string templatePath() const override {
        return "projects.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};