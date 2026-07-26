#pragma once

#include "features/project_types/data/portal_project_type_update.hpp"
#include "features/project_types/providers/project_type_provider.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>

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

        return drogular::ActionResult::redirect(
            "/project-types?success=project_type_updated"
        );
    }
};