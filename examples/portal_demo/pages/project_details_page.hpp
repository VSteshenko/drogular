#pragma once

#include "../providers/project_type_provider.hpp"
#include "../providers/project_provider.hpp"
#include "../ui/portal_page_support.hpp"
#include "../providers/role_provider.hpp"
#include "data/portal_schema.hpp"

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
            context.requireService<PortalProjectProvider>();

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

        const auto schema =
            PortalSchema::projects();

        context.set(
            "idLabel",
            context.translate(schema.fieldLabelKey("id"))
        );
        context.set(
            "titleLabel",
            context.translate(schema.fieldLabelKey("title"))
        );
        context.set(
            "statusLabel",
            context.translate(schema.fieldLabelKey("status"))
        );
        context.set(
            "ownerLabel",
            context.translate(schema.fieldLabelKey("ownerId"))
        );
        context.set(
            "typeLabel",
            context.translate(schema.fieldLabelKey("projectTypeId"))
        );

        context.set("hasProjectsSuccess", !projectsSuccess.empty());
        context.set("alertMessage", projectsSuccess);
        context.set("projectId", project->id);
        context.set("projectTitle", project->title);
        context.set("projectStatus", project->status);

        const auto owner =
            repository->owner(*project);

        context.set(
            "projectOwnerName",
            owner.has_value()
                ? owner->username
                : std::string("")
        );

        context.set(
            "hasProjectOwner",
            owner.has_value()
        );

        auto roleProvider =
            context.requireService<PortalRoleProvider>();

        const auto ownerRole =
            owner.has_value()
                ? roleProvider->findByCode(owner->role)
                : std::nullopt;

        context.set(
            "projectOwnerRole",
            ownerRole.has_value()
                ? ownerRole->title
                : std::string("")
        );

        auto projectTypeProvider =
            context.requireService<PortalProjectTypeProvider>();

        const auto projectType =
            projectTypeProvider->findById(
                project->projectTypeId
            );

        context.set(
            "hasProjectType",
            projectType.has_value()
        );

        context.set(
            "projectTypeTitle",
            projectType.has_value()
                ? projectType->title
                : std::string("")
        );
    }

    std::string templatePath() const override {
        return "project_details.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};