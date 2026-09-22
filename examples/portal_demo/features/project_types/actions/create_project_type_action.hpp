#pragma once

#include "features/project_types/data/portal_project_type_create.hpp"
#include "features/project_types/providers/project_type_provider.hpp"
#include "features/project_types/ui/portal_project_type_create_form_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/action_renderer.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/component.hpp>
#include <drogular/url.hpp>

class PortalCreateProjectTypeAction final
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

        if (const auto result =
                drogular::ActionAuthSupport::requireSessionValue(
                    context,
                    "role",
                    "admin",
                    "/project-types?error=access_denied"
                )) {
            return *result;
        }

        const auto validation =
            drogular::FormValidator(context)
                .required("code")
                .minLength("code", 2)
                .required("title")
                .minLength("title", 2)
                .validate();

        const auto code =
            context.form<std::string>("code")
                .value_or("");

        const auto title =
            context.form<std::string>("title")
                .value_or("");

        if (!validation.valid()) {
            if (context.isInteraction()) {
                return renderForm(
                    context,
                    code,
                    title,
                    "validation",
                    "",
                    drogon::k422UnprocessableEntity
                );
            }

            return drogular::ActionResult::redirect(
                "/project-types?error=validation"
                "&code=" +
                drogular::Url::encode(code) +
                "&title=" +
                drogular::Url::encode(title)
            );
        }

        auto projectTypes =
            context.requireService<
                PortalProjectTypeProvider
            >();

        const auto existing =
            projectTypes->all();

        const auto duplicate =
            std::any_of(
                existing.begin(),
                existing.end(),
                [&code](const PortalProjectType& type) {
                    return type.code == code;
                }
            );

        if (duplicate) {
            if (context.isInteraction()) {
                return renderForm(
                    context,
                    code,
                    title,
                    "duplicate_code",
                    "",
                    drogon::k422UnprocessableEntity
                );
            }

            return drogular::ActionResult::redirect(
                "/project-types?error=duplicate_code"
                "&code=" +
                drogular::Url::encode(code) +
                "&title=" +
                drogular::Url::encode(title)
            );
        }

        PortalProjectTypeCreate input;

        input.code =
            context.requireForm<std::string>("code");

        input.title =
            context.requireForm<std::string>("title");

        projectTypes->create(input);

        if (context.isInteraction()) {
            return renderForm(
                context,
                "",
                "",
                "",
                "project_type_created"
            );
        }

        return drogular::ActionResult::redirect(
            "/project-types?success=project_type_created"
        );
    }

private:
    class CreateFormComponent final
        : public drogular::TemplateComponent
    {
    public:
        std::string templatePath() const override {
            return "fragments/project_type_create_form.html";
        }
    };

    static drogular::ActionResult renderForm(
        drogular::ActionContext& context,
        const std::string& code,
        const std::string& title,
        const std::string& error,
        const std::string& success,
        drogon::HttpStatusCode status = drogon::k200OK
    ) {
        return drogular::ActionRenderer::render<
            CreateFormComponent
        >(
            context,
            [&](drogular::RenderContext& renderContext) {
                PortalPageSupport::apply(
                    renderContext,
                    "project_types.page.title"
                );
                PortalProjectTypeCreateFormSupport::apply(
                    renderContext,
                    code,
                    title,
                    error,
                    success
                );
            },
            status
        );
    }
};