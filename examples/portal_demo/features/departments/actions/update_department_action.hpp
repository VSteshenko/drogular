#pragma once

#include "features/departments/providers/department_provider.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/action_auth_support.hpp>
#include <drogular/form_validator.hpp>

#include <cstdlib>

class PortalUpdateDepartmentAction final
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

        const auto id =
            std::atoi(context.requireRouteParam("id").c_str());

        const auto validation =
            drogular::FormValidator(context)
                .required("name")
                .minLength("name", 2)
                .required("managerId")
                .validate();

        if (!validation.valid()) {
            return drogular::ActionResult::redirect(
                "/departments/" +
                    std::to_string(id) +
                    "/edit?error=validation"
            );
        }

        auto provider =
            context.requireService<
                PortalDepartmentProvider
            >();
        const auto name =
            context.requireForm<std::string>(
                "name"
            );
        if (provider->exists(name, id)) {
            return drogular::ActionResult::redirect(
                "/departments/" +
                    std::to_string(id) +
                    "/edit?error=duplicate");
        }

        PortalDepartmentUpdate input;

        input.id = id;
        input.name = name;
        input.description =
            context.form<std::string>("description").value_or("");
        input.managerId =
            context.requireForm<int>("managerId");
        input.isActive =
            context.form<std::string>("isActive").value_or("") == "on";

        provider->update(input);

        return drogular::ActionResult::redirect(
            "/departments?success=updated"
        );
    }
};