#pragma once

#include "../portal_project_repository.hpp"
#include "../ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <cstdlib>

class PortalProjectEditPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "projects.edit.title"
        );

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        const auto id =
            std::atoi(context.requireRouteParam("id").c_str());

        auto repository =
            context.requireService<PortalProjectRepository>();

        const auto project =
            repository->findById(id);

        context.set("projectNotFound", !project.has_value());

        if (!project.has_value()) {
            return;
        }

        context.set("projectId", project->id);
        context.set("projectTitle", project->title);
        context.set("projectStatus", project->status);
    }

    std::string templatePath() const override {
        return "project_edit.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};