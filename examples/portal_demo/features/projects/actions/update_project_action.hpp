#pragma once

#include "features/projects/providers/project_provider.hpp"
#include "features/projects/ui/portal_project_edit_form_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/url.hpp>
#include <drogular/component.hpp>
#include <drogular/render_context.hpp>

#include <string>

class PortalUpdateProjectAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        if (const auto result =
                drogular::ActionAuthSupport::requireAuthentication(context)) {
            return *result;
        }

        const auto id =
            std::atoi(context.requireRouteParam("id").c_str());

        auto repository =
            context.requireService<PortalProjectProvider>();

        const auto project =
            repository->findById(id);

        if (!project.has_value()) {
            return drogular::ActionResult::redirect(
                "/projects?error=not_found"
            );
        }

        const auto title =
            context.form<std::string>("title").value_or("");

        const auto validation =
            drogular::FormValidator(context)
                .minLength("title", 2)
                .validate();

        if (!validation.valid()) {
            if (isInteraction(context)) {
                return renderForm(
                    context,
                    *project,
                    title,
                    context.form<int>("projectTypeId")
                        .value_or(project->projectTypeId),
                    context.form<std::string>("status")
                        .value_or(project->status),
                    "validation",
                    "",
                    drogon::k422UnprocessableEntity
                );
            }

            return drogular::ActionResult::redirect(
                "/projects/" + std::to_string(id) +
                "/edit?error=validation&title=" +
                drogular::Url::encode(title)
            );
        }


        PortalProjectUpdate input;
        input.id = id;

        if (!title.empty()) {
            input.title = title;
        }

        const auto status =
            context.form<std::string>("status").value_or("");

        if (!status.empty()) {
            input.status = status;
        }

        const auto projectTypeId =
            context.form<int>("projectTypeId").value_or(0);

        if (projectTypeId > 0) {
            input.projectTypeId =
                projectTypeId;
        }

        const auto updated =
            repository->update(
                input
            );

        if (updated.id != id) {
            return drogular::ActionResult::redirect(
                "/projects/" + std::to_string(id) +
                "/edit?error=not_found"
            );
        }

        if (isInteraction(context)) {
            return renderForm(
                context,
                updated,
                updated.title,
                updated.projectTypeId,
                updated.status,
                "",
                "project_updated"
            );
        }

        return drogular::ActionResult::redirect(
            "/projects/" + std::to_string(id) +
            "?success=project_updated"
        );
    }

private:
    static bool isInteraction(
        const drogular::ActionContext& context
    ) {
        const auto request = context.request();
        return request != nullptr &&
            request->getHeader("X-Drogular-Interaction") == "true";
    }

    class EditFormComponent final
        : public drogular::TemplateComponent
    {
    public:
        std::string templatePath() const override {
            return "fragments/project_edit_form.html";
        }
    };

    static drogular::ActionResult renderForm(
        drogular::ActionContext& context,
        const PortalProject& project,
        const std::string& title,
        int projectTypeId,
        const std::string& status,
        const std::string& error,
        const std::string& success,
        drogon::HttpStatusCode responseStatus = drogon::k200OK
    ) {
        drogular::RenderContext renderContext;
        renderContext.setServices(context.services());
        renderContext.setRequest(context.request());

        PortalPageSupport::apply(renderContext, "projects.edit.title");
        PortalProjectEditFormSupport::apply(
            renderContext,
            project,
            title,
            projectTypeId,
            status,
            error,
            success
        );

        EditFormComponent component;
        component.onInit(renderContext);
        auto html = component.render(renderContext);
        component.onDestroy(renderContext);

        return drogular::ActionResult::html(
            std::move(html),
            responseStatus
        );
    }
};