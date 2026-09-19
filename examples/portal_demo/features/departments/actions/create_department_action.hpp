#pragma once

#include "features/departments/providers/department_provider.hpp"
#include "features/departments/ui/portal_department_create_form_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/action_auth_support.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/url.hpp>
#include <drogular/component.hpp>
#include <drogular/render_context.hpp>

class PortalCreateDepartmentAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        if (const auto result =
            drogular::ActionAuthSupport::requireAuthentication(context)
            ) {
            return *result;
        }

        if (const auto result =
            drogular::ActionAuthSupport::requireSessionValue(
                context, "role",
                "admin",
                "/departments?error=access_denied")
            ) {
            return *result;
        }

        const auto validation =
            drogular::FormValidator(context)
                .required("name")
                .minLength("name", 2)
                .required("managerId")
                .validate();

        const auto name =
            context.form<std::string>("name").value_or("");
        const auto description =
            context.form<std::string>("description").value_or("");
        const auto managerId =
            context.form<std::string>("managerId").value_or("");
        const auto isActive =
            context.form<std::string>("isActive").value_or("") == "on";

        if (!validation.valid()) {
            if (isInteraction(context)) {
                return renderForm(
                    context,
                    name,
                    description,
                    managerId,
                    isActive,
                    "validation",
                    "",
                    drogon::k422UnprocessableEntity
                );
            }

            return drogular::ActionResult::redirect(
                "/departments?error=validation&name=" +
                    drogular::Url::encode(name)
            );
        }

        auto provider =
            context.requireService<
                PortalDepartmentProvider
            >();

        if (provider->exists(name)) {
            if (isInteraction(context)) {
                return renderForm(
                    context,
                    name,
                    description,
                    managerId,
                    isActive,
                    "duplicate",
                    "",
                    drogon::k422UnprocessableEntity
                );
            }

            return drogular::ActionResult::redirect(
                "/departments?error=duplicate&name=" +
                    drogular::Url::encode(name)
            );
        }

        PortalDepartmentCreate input;
        input.name = name;
        input.description = description;
        input.managerId = context.requireForm<int>("managerId");
        input.isActive = isActive;

        provider->create(input);

        if (isInteraction(context)) {
            return renderForm(
                context,
                "",
                "",
                "",
                true,
                "",
                "created"
            );
        }

        return drogular::ActionResult::redirect(
            "/departments?success=created"
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

    class CreateFormComponent final
        : public drogular::TemplateComponent
    {
    public:
        std::string templatePath() const override {
            return "fragments/department_create_form.html";
        }
    };

    static drogular::ActionResult renderForm(
        drogular::ActionContext& context,
        const std::string& name,
        const std::string& description,
        const std::string& managerId,
        bool isActive,
        const std::string& error,
        const std::string& success,
        drogon::HttpStatusCode responseStatus = drogon::k200OK
    ) {
        drogular::RenderContext renderContext;
        renderContext.setServices(context.services());
        renderContext.setRequest(context.request());

        PortalPageSupport::apply(renderContext, "departments.title");
        PortalDepartmentCreateFormSupport::apply(
            renderContext,
            name,
            description,
            managerId,
            isActive,
            error,
            success
        );

        CreateFormComponent component;
        component.onInit(renderContext);
        auto html = component.render(renderContext);
        component.onDestroy(renderContext);

        return drogular::ActionResult::html(
            std::move(html),
            responseStatus
        );
    }
};