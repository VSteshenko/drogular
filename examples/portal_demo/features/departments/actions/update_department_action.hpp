#pragma once

#include "features/departments/providers/department_provider.hpp"
#include "features/departments/ui/portal_department_edit_form_support.hpp"
#include "features/departments/ui/portal_department_navigation_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/action_renderer.hpp>
#include <drogular/action_auth_support.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/component.hpp>

#include <cstdlib>
#include <string>

class PortalUpdateDepartmentAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        if (const auto r =
            drogular::ActionAuthSupport::requireAuthentication(context)
        ) {
            return *r;
        }
        if (const auto r =
            drogular::ActionAuthSupport::requireSessionValue(
                context,
                "role",
                "admin",
                "/departments?error=access_denied")
        ) {
            return *r;
        }
        const auto id = std::atoi(context.requireRouteParam("id").c_str());
        auto provider =
            context.requireService<PortalDepartmentProvider>();
        const auto department = provider->findById(id);
        if (!department) {
            return drogular::ActionResult::redirect("/departments?error=not_found");
        }

        const auto name = context.form<std::string>("name").value_or("");
        const auto description = context.form<std::string>("description").value_or("");
        const auto managerId = context.form<int>("managerId").value_or(department->managerId);
        const auto isActive = context.form<std::string>("isActive").value_or("")=="on";
        const auto returnUrl = PortalDepartmentNavigationSupport::departmentsReturnUrl(
            context.form<std::string>("returnUrl").value_or("/departments")
        );
        const auto validation = drogular::FormValidator(context)
            .required("name")
            .minLength("name", 2)
            .required("managerId")
            .validate();
        if (!validation.valid()) {
            if (isInteraction(context)) {
                return renderForm(
                    context,
                    *department,
                    name,
                    description,
                    managerId,
                    isActive,
                    "validation",
                    "",returnUrl,
                    drogon::k422UnprocessableEntity
                );
            }

            return drogular::ActionResult::redirect(
                PortalDepartmentNavigationSupport::editUrl(id, returnUrl) + "&error=validation"
            );
        }
        if (provider->exists(name, id)) {
            if (isInteraction(context)) {
                return renderForm(
                    context,
                    *department,
                    name,
                    description,
                    managerId,
                    isActive,
                    "duplicate",
                    "",
                    returnUrl,
                    drogon::k422UnprocessableEntity
                );
            }

            return drogular::ActionResult::redirect(
                PortalDepartmentNavigationSupport::editUrl(id, returnUrl) + "&error=duplicate"
            );
        }

        PortalDepartmentUpdate input;
        input.id = id;
        input.name = name;
        input.description = description;
        input.managerId = managerId;
        input.isActive = isActive;
        provider->update(input);

        const auto updated = provider->findById(id);
        if (!updated) {
            return drogular::ActionResult::redirect(
                "/departments?error=not_found"
            );
        }
        if (isInteraction(context)) {
            return renderForm(
                context,
                *updated,
                updated->name,
                updated->description,
                updated->managerId,
                updated->isActive,
                "",
                "updated",
                returnUrl
            );
        }

        return drogular::ActionResult::redirect(
            PortalDepartmentNavigationSupport::detailsUrl(id, returnUrl) + "&success=updated"
        );
    }

private:
    static bool isInteraction(const drogular::ActionContext& context) {
        const auto r = context.request();
        return r && r->getHeader("X-Drogular-Interaction")=="true";
    }

    class EditFormComponent final
        : public drogular::TemplateComponent
    {
    public:
        std::string templatePath() const override {
            return "fragments/department_edit_form.html";
        }
    };

    static drogular::ActionResult renderForm(
        drogular::ActionContext& context,
        const PortalDepartment& department,
        const std::string& name,
        const std::string& description,
        int managerId,
        bool isActive,
        const std::string& error,
        const std::string& success,
        const std::string& returnUrl,
        drogon::HttpStatusCode status = drogon::k200OK
    ) {
        return drogular::ActionRenderer::render<
            EditFormComponent
        >(
            context,
            [&](drogular::RenderContext& renderContext) {
                PortalPageSupport::apply(
                    renderContext,
                    "departments.edit.title"
                );
                PortalDepartmentEditFormSupport::apply(
                    renderContext,
                    department,
                    name,
                    description,
                    managerId,
                    isActive,
                    error,
                    success,
                    returnUrl
                );
            },
            status
        );
    }
};