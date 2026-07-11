#pragma once

#include "../providers/project_provider.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/url.hpp>

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

        const auto title =
            context.form<std::string>("title").value_or("");

        const auto validation =
            drogular::FormValidator(context)
                .minLength("title", 2)
                .validate();

        if (!validation.valid()) {
            return drogular::ActionResult::redirect(
                "/projects/" + std::to_string(id) +
                "/edit?error=validation&title=" +
                drogular::Url::encode(title)
            );
        }

        auto repository =
            context.requireService<PortalProjectProvider>();

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

        return drogular::ActionResult::redirect(
            "/projects/" + std::to_string(id) +
            "?success=project_updated"
        );
    }
};