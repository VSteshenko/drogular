#pragma once

#include "features/departments/providers/department_provider.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/action_auth_support.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/url.hpp>

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

        if (!validation.valid()) {
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
            return drogular::ActionResult::redirect(
                "/departments?error=duplicate&name=" +
                    drogular::Url::encode(name)
            );
        }

        PortalDepartmentCreate input;
        input.name = name;
        input.description = context.form<std::string>("description").value_or("");
        input.managerId = context.requireForm<int>("managerId");
        input.isActive = context.form<std::string>("isActive").value_or("") == "on";

        provider->create(input);

        return drogular::ActionResult::redirect(
            "/departments?success=created"
        );
    }
};