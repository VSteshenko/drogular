#pragma once

#include "../providers/project_provider.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/url.hpp>

#include <cstdlib>
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

        const auto status =
            context.form<std::string>("status").value_or("");

        const auto validation =
            drogular::FormValidator(context)
                .required("title")
                .minLength("title", 2)
                .required("status")
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

        PortalProject project;
        project.id = id;
        project.title = context.requireForm<std::string>("title");
        project.status = context.requireForm<std::string>("status");

        const auto updated =
            repository->update(
                project
            );

        if (!updated) {
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