#pragma once

#include "../providers/project_provider.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>
#include <drogular/url.hpp>
#include <drogular/action_auth_support.hpp>

class PortalCreateProjectAction final
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

        const auto validation =
            drogular::FormValidator(context)
                .required("title")
                .minLength("title", 2)
                .required("status")
                .validate();

        const auto title =
            context.form<std::string>("title")
                .value_or("");

        if (!validation.valid()) {
            // validation failed
            return drogular::ActionResult::redirect(
                "/projects?error=validation&title=" +
                drogular::Url::encode(title)
            );
        }

        auto repository =
            context.requireService<PortalProjectProvider>();

        PortalProject project;
        project.title = context.requireForm<std::string>("title");
        project.status = context.requireForm<std::string>("status");

        repository->create(
            project
        );

        // success
        return drogular::ActionResult::redirect(
            "/projects?success=project_created"
        );
    }
};