#pragma once

#include "features/project_types/data/portal_project_type_update.hpp"
#include "features/project_types/providers/project_type_provider.hpp"
#include "features/project_types/ui/portal_project_type_edit_form_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/action_renderer.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/component.hpp>

#include <algorithm>
#include <cstdlib>
#include <string>

class PortalUpdateProjectTypeAction final
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

        const auto id =
            std::atoi(
                context.requireRouteParam("id").c_str()
            );

        const auto code =
            context.form<std::string>("code");

        const auto title =
            context.form<std::string>("title");

        drogular::FormValidator validator(context);

        if (code.has_value()) {
            validator.required("code");
            validator.minLength("code", 2);
        }

        if (title.has_value()) {
            validator.required("title");
            validator.minLength("title", 2);
        }

        const auto validation =
            validator.validate();

        if (!validation.valid()) {
            if (isInteraction(context)) {
                return renderForm(
                    context,
                    id,
                    code.value_or(""),
                    title.value_or(""),
                    "validation",
                    "",
                    drogon::k422UnprocessableEntity
                );
            }

            return drogular::ActionResult::redirect(
                "/project-types/" +
                std::to_string(id) +
                "/edit?error=validation"
            );
        }

        auto projectTypes =
            context.requireService<
                PortalProjectTypeProvider
            >();

        const auto all =
            projectTypes->all();

        const auto duplicate =
            std::any_of(
                all.begin(),
                all.end(),
                [id, &code](
                    const PortalProjectType& type
                ) {
                    return type.id != id &&
                           type.code == code;
                }
            );

        if (duplicate) {
            if (isInteraction(context)) {
                return renderForm(
                    context,
                    id,
                    code.value_or(""),
                    title.value_or(""),
                    "duplicate_code",
                    "",
                    drogon::k422UnprocessableEntity
                );
            }

            return drogular::ActionResult::redirect(
                "/project-types/" +
                std::to_string(id) +
                "/edit?error=duplicate_code"
            );
        }

        PortalProjectTypeUpdate input;
        input.id = id;

        if (code.has_value()) {
            input.code = *code;
        }

        if (title.has_value()) {
            input.title = *title;
        }

        const auto updated =
            projectTypes->update(input);

        if (updated.id != id) {
            return drogular::ActionResult::redirect(
                "/project-types?error=not_found"
            );
        }

        if (isInteraction(context)) {
            return renderForm(
                context,
                updated.id,
                updated.code,
                updated.title,
                "",
                "project_type_updated"
            );
        }

        return drogular::ActionResult::redirect(
            "/project-types?success=project_type_updated"
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
            return "fragments/project_type_edit_form.html";
        }
    };

    static drogular::ActionResult renderForm(
        drogular::ActionContext& context,
        int projectTypeId,
        const std::string& code,
        const std::string& title,
        const std::string& error,
        const std::string& success,
        drogon::HttpStatusCode status = drogon::k200OK
    ) {
        return drogular::ActionRenderer::render<
            EditFormComponent
        >(
            context,
            [&](drogular::RenderContext& renderContext) {
                PortalPageSupport::apply(
                    renderContext,
                    "project_types.edit_page.title"
                );
                PortalProjectTypeEditFormSupport::apply(
                    renderContext,
                    projectTypeId,
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