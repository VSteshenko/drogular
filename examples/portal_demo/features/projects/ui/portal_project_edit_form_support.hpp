#pragma once

#include "features/projects/data/portal_project.hpp"
#include "features/project_types/providers/project_type_provider.hpp"
#include "features/localization/support/portal_error_translator.hpp"
#include "data/portal_schema.hpp"

#include <drogular/render_context.hpp>

class PortalProjectEditFormSupport final {
public:
    static void apply(
        drogular::RenderContext& context,
        const PortalProject& project,
        const std::string& title,
        int projectTypeId,
        const std::string& status,
        const std::string& error = {},
        const std::string& success = {}
    ) {
        const auto schema = PortalSchema::projects();
        const auto projectsError =
            PortalErrorTranslator::projectsError(context, error);
        const auto projectsSuccess =
            PortalErrorTranslator::projectsSuccess(context, success);

        context.set("hasProjectsError", !projectsError.empty());
        context.set("hasProjectsSuccess", !projectsSuccess.empty());
        context.set(
            "alertMessage",
            !projectsError.empty() ? projectsError : projectsSuccess
        );

        context.set("projectId", project.id);
        context.set("projectTitle", title);
        context.set("projectStatus", status);

        context.set("projectTitleRequired", schema.fieldRequired("title"));
        context.set("projectTypeRequired", schema.fieldRequired("projectTypeId"));
        context.set("projectStatusRequired", schema.fieldRequired("status"));

        auto projectTypes =
            context.requireService<PortalProjectTypeProvider>();

        Json::Value options(Json::arrayValue);
        for (const auto& type : projectTypes->all()) {
            Json::Value option(Json::objectValue);
            option["value"] = type.id;
            option["label"] = type.title;
            option["selected"] = type.id == projectTypeId;
            options.append(std::move(option));
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

        context.set("isActive", status == "active");
        context.set("isPaused", status == "paused");
        context.set("isDone", status == "done");
    }
};