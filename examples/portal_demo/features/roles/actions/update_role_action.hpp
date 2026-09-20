#pragma once

#include "features/roles/data/portal_role_update.hpp"
#include "features/roles/providers/role_provider.hpp"
#include "features/roles/ui/portal_role_edit_form_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/component.hpp>
#include <drogular/render_context.hpp>

#include <algorithm>
#include <cstdlib>
#include <string>

class PortalUpdateRoleAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        if (const auto result =
                drogular::ActionAuthSupport::
                    requireAuthentication(context)) {
            return *result;
        }

        if (const auto result =
                drogular::ActionAuthSupport::
                    requireSessionValue(
                        context,
                        "role",
                        "admin",
                        "/roles?error=access_denied"
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
                "/roles/" +
                std::to_string(id) +
                "/edit?error=validation"
            );
        }

        auto roles =
            context.requireService<PortalRoleProvider>();

        const auto all =
            roles->all();

        const auto duplicate =
            std::any_of(
                all.begin(),
                all.end(),
                [id, &code](const PortalRole& role) {
                    return code.has_value() &&
                           role.id != id &&
                           role.code == *code;
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
                "/roles/" +
                std::to_string(id) +
                "/edit?error=duplicate_code"
            );
        }

        PortalRoleUpdate input;
        input.id = id;

        if (code.has_value()) {
            input.code = *code;
        }

        if (title.has_value()) {
            input.title = *title;
        }

        const auto updated =
            roles->update(input);

        if (updated.id != id) {
            return drogular::ActionResult::redirect(
                "/roles?error=not_found"
            );
        }

        if (isInteraction(context)) {
            return renderForm(
                context,
                updated.id,
                updated.code,
                updated.title,
                "",
                "role_updated"
            );
        }

        return drogular::ActionResult::redirect(
            "/roles?success=role_updated"
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
            return "fragments/role_edit_form.html";
        }
    };

    static drogular::ActionResult renderForm(
        drogular::ActionContext& context,
        int roleId,
        const std::string& code,
        const std::string& title,
        const std::string& error,
        const std::string& success,
        drogon::HttpStatusCode status = drogon::k200OK
    ) {
        drogular::RenderContext renderContext;
        renderContext.setServices(context.services());
        renderContext.setRequest(context.request());

        PortalPageSupport::apply(
            renderContext,
            "roles.edit_page.title"
        );
        PortalRoleEditFormSupport::apply(
            renderContext,
            roleId,
            code,
            title,
            error,
            success
        );

        EditFormComponent component;
        component.onInit(renderContext);
        auto html = component.render(renderContext);
        component.onDestroy(renderContext);

        return drogular::ActionResult::html(
            std::move(html),
            status
        );
    }
};