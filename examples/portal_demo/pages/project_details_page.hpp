#pragma once

#include "../portal_project_repository.hpp"
#include "../ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <cstdlib>

class PortalProjectDetailsPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "projects.details.title"
        );

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        const auto id =
            std::atoi(
                context.requireRouteParam("id").c_str()
            );

        auto repository =
            context.requireService<PortalProjectRepository>();

        const auto project =
            repository->findById(id);

        context.set(
            "projectNotFound",
            !project.has_value()
        );

        if (!project.has_value()) {
            return;
        }

        const auto request =
            context.request();

        const auto success =
            request != nullptr
                ? request->getParameter("success")
                : std::string("");

        const auto projectsSuccess =
            PortalErrorTranslator::projectsSuccess(
                context,
                success
            );

        context.set("hasProjectsSuccess", !projectsSuccess.empty());
        context.set("alertMessage", projectsSuccess);
        context.set("projectId", project->id);
        context.set("projectTitle", project->title);
        context.set("projectStatus", project->status);
    }

    std::string templatePath() const override {
        return "project_details.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};