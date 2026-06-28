#pragma once

#include "../portal_project_repository.hpp"

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