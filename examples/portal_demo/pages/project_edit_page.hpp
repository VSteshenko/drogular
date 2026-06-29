#pragma once

#include "../portal_memory_project_provider.hpp"
#include "../ui/portal_page_support.hpp"
#include "../localization/portal_error_translator.hpp"

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

        const auto request =
            context.request();

        const auto error =
            request != nullptr
                ? request->getParameter("error")
                : std::string("");

        const auto projectsError =
            PortalErrorTranslator::projectsError(
                context,
                error
            );

        context.set("hasProjectsError", !projectsError.empty());
        context.set("alertMessage", projectsError);

        context.set("projectId", project->id);
        context.set("projectTitle", project->title);
        context.set("projectStatus", project->status);

        context.set("isActive", project->status == "active");
        context.set("isPaused", project->status == "paused");
        context.set("isDone", project->status == "done");
    }

    std::string templatePath() const override {
        return "project_edit.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};