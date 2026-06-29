#pragma once

#include "../portal_memory_project_provider.hpp"
#include "../ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

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

        const auto projectsError =
            PortalErrorTranslator::projectsError(
                context,
                error
            );

        const auto projectsSuccess =
            PortalErrorTranslator::projectsSuccess(
                context,
                success
            );

        context.set("hasProjectsError", !projectsError.empty());
        context.set("hasProjectsSuccess", !projectsSuccess.empty());
        context.set("alertMessage", !projectsError.empty() ? projectsError : projectsSuccess);
        context.set("createProjectTitle", createTitle);

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        auto repository =
            context.requireService<PortalProjectProvider>();

        Json::Value projects(Json::arrayValue);

        for (const auto& project : repository->all()) {
            Json::Value value;
            value["id"] = project.id;
            value["title"] = project.title;
            value["status"] = project.status;
            projects.append(value);
        }

        context.set("projects", projects);
    }

    std::string templatePath() const override
    {
        return "projects.html";
    }

    std::string layoutPath() const override
    {
        return "layouts/main.html";
    }
};