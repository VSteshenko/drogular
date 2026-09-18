#pragma once

#include "features/projects/providers/project_provider.hpp"
#include "features/projects/ui/portal_project_edit_form_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <cstdlib>
#include <string>

class PortalProjectEditPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(context, "projects.edit.title");

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        const auto id =
            std::atoi(context.requireRouteParam("id").c_str());

        auto repository =
            context.requireService<PortalProjectProvider>();

        const auto project =
            repository->findById(id);

        context.set("projectNotFound", !project.has_value());

        if (!project.has_value()) {
            return;
        }

        const auto request = context.request();
        const auto error =
            request != nullptr
                ? request->getParameter("error")
                : std::string("");

        PortalProjectEditFormSupport::apply(
            context,
            *project,
            project->title,
            project->projectTypeId,
            project->status,
            error
        );
    }

    std::string templatePath() const override {
        return "project_edit.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};