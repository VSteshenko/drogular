#pragma once

#include "../portal_project_repository.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>

class PortalCreateProjectAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto session =
            context.existingSession();

        if (session == nullptr) {
            return drogular::ActionResult::redirect(
                "/login"
            );
        }

        const auto title =
            context.form<std::string>("title")
            .value_or("");

        const auto validation =
            drogular::FormValidator(context)
                .required("title")
                .minLength("title", 2)
                .required("status")
                .validate();

        if (!validation.valid()) {
            // validation failed
            return drogular::ActionResult::redirect(
                "/projects?error=validation&title=" + title
            );
        }

        auto repository =
            context.requireService<PortalProjectRepository>();

        repository->create(
            context.requireForm<std::string>("title"),
            context.requireForm<std::string>("status")
        );

        // success
        return drogular::ActionResult::redirect(
            "/projects?success=project_created"
        );
    }
};