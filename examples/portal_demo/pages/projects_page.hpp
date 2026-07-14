#pragma once

#include "../providers/project_provider.hpp"
#include "../providers/project_type_provider.hpp"
#include "../ui/portal_page_support.hpp"
#include "../data/portal_schema.hpp"
#include "../localization/portal_error_translator.hpp"

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

        const auto schema =
            PortalSchema::projects();

        context.set(
            "projectTitleRequired",
            schema.fieldRequired("title")
        );

        context.set(
            "projectTypeRequired",
            schema.fieldRequired("projectTypeId")
        );

        context.set(
            "projectStatusRequired",
            schema.fieldRequired("status")
        );

        auto projectTypes =
            context.requireService<PortalProjectTypeProvider>();

        Json::Value options(Json::arrayValue);

        for (const auto& type : projectTypes->all()) {
            Json::Value option(Json::objectValue);

            option["value"] = type.id;
            option["label"] = type.title;

            options.append(option);
        }

        context.set("projectTypeOptions", options);

        context.set(
            "projectsTitleLabel",
            context.translate(schema.fieldLabelKey("title"))
        );
        context.set(
            "typeLabel",
            context.translate(schema.fieldLabelKey("projectTypeId"))
        );
        context.set(
            "projectsStatusLabel",
            context.translate(schema.fieldLabelKey("status"))
        );

        const auto search =
            request != nullptr
                ? request->getParameter("search")
                : std::string("");

        context.set("projectSearch", search);

        auto repository =
            context.requireService<PortalProjectProvider>();

        Json::Value projects(Json::arrayValue);

        PortalProjectFilter filter;

        if (!search.empty()) {
            filter.search = search;
        }

        for (const auto& project :
             repository->search(filter)) {
            Json::Value value;

            value["id"] = project.id;
            value["title"] = project.title;
            value["status"] = project.status;

            projects.append(value);
        }

        context.set("projects", projects);
        context.set("hasProjects", !projects.empty());
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