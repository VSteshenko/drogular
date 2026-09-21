#pragma once

#include "features/projects/providers/project_provider.hpp"
#include "features/projects/ui/portal_project_create_form_support.hpp"
#include "features/auth/support/portal_auth_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/action_renderer.hpp>
#include <drogular/action_result.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/url.hpp>
#include <drogular/action_auth_support.hpp>
#include <drogular/component.hpp>

class PortalProjectCreateFormComponent final
    : public drogular::TemplateComponent
{
public:
    std::string templatePath() const override {
        return "fragments/project_create_form.html";
    }
};

class PortalCreateProjectAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        if (const auto result =
                drogular::ActionAuthSupport::requireAuthentication(
                    context
                )) {
            return *result;
        }

        const auto currentUser =
            PortalAuthSupport::currentUser(context);

        if (!currentUser.has_value()) {
            return drogular::ActionResult::redirect(
                "/login"
            );
        }

        const auto validation =
            drogular::FormValidator(context)
                .required("title")
                .minLength("title", 2)
                .required("projectTypeId")
                .validate();

        const auto title =
            context.form<std::string>("title")
                .value_or("");
        const auto projectTypeId =
            context.form<std::string>("projectTypeId")
                .value_or("");
        const auto status =
            context.form<std::string>("status")
                .value_or("active");

        if (!validation.valid()) {
            if (isInteraction(context)) {
                return renderForm(
                    context,
                    title,
                    projectTypeId,
                    status,
                    "validation",
                    "",
                    drogon::k422UnprocessableEntity
                );
            }

            return drogular::ActionResult::redirect(
                "/projects?error=validation&title=" +
                drogular::Url::encode(title)
            );
        }

        auto repository =
            context.requireService<PortalProjectProvider>();

        PortalProjectCreate input;
        input.title = context.requireForm<std::string>("title");
        input.projectTypeId = context.requireForm<int>("projectTypeId");
        input.status = status;

        repository->create(
            input,
            currentUser->id
        );

        if (isInteraction(context)) {
            return renderForm(
                context,
                "",
                "",
                "active",
                "",
                "project_created"
            );
        }

        return drogular::ActionResult::redirect(
            "/projects?success=project_created"
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

    static drogular::ActionResult renderForm(
        drogular::ActionContext& context,
        const std::string& title,
        const std::string& projectTypeId,
        const std::string& status,
        const std::string& error,
        const std::string& success,
        drogon::HttpStatusCode responseStatus = drogon::k200OK
    ) {
        return drogular::ActionRenderer::render<
            PortalProjectCreateFormComponent
        >(
            context,
            [&](drogular::RenderContext& renderContext) {
                PortalPageSupport::apply(
                    renderContext,
                    "projects.title"
                );
                PortalProjectCreateFormSupport::apply(
                    renderContext,
                    title,
                    projectTypeId,
                    status,
                    error,
                    success
                );
            },
            responseStatus
        );
    }
};