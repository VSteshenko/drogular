#pragma once

#include "features/project_types/providers/project_type_provider.hpp"
#include "features/localization/support/portal_error_translator.hpp"
#include "data/portal_schema.hpp"

#include <drogular/render_context.hpp>

class PortalProjectCreateFormSupport final {
public:
    static void apply(
        drogular::RenderContext& context,
        const std::string& title = {},
        const std::string& projectTypeId = {},
        const std::string& status = "active",
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
        context.set("createProjectTitle", title);
        context.set("createProjectStatusActive", status == "active");
        context.set("createProjectStatusPaused", status == "paused");
        context.set("createProjectStatusDone", status == "done");

        context.set("projectTitleRequired", schema.fieldRequired("title"));
        context.set("projectTypeRequired", schema.fieldRequired("projectTypeId"));
        context.set("projectStatusRequired", schema.fieldRequired("status"));

        auto projectTypes = context.requireService<PortalProjectTypeProvider>();
        const auto allProjectTypes = projectTypes->all();
        Json::Value options(Json::arrayValue);

        for (const auto& type : allProjectTypes) {
            Json::Value option(Json::objectValue);
            option["value"] = type.id;
            option["label"] = type.title;
            option["selected"] = projectTypeId == std::to_string(type.id);
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
    }
};