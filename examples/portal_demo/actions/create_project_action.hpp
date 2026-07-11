#pragma once

#include "../providers/project_provider.hpp"
#include "../auth/portal_auth_support.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/action_result.hpp>
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

        const auto currentUser =
            PortalAuthSupport::currentUser(context);

        if (!currentUser.has_value()) {
            return drogular::ActionResult::redirect(
                "/login"
            );
        }

        const auto validation =
            drogular::FormValidator(context)
                .required("title")
                .minLength("title", 2)
                .required("projectTypeId")
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

        PortalProjectCreate input;
        input.title = context.requireForm<std::string>("title");
        input.projectTypeId = context.requireForm<int>("projectTypeId");
        input.status = context
            .form<std::string>("status")
            .value_or("active");

        repository->create(
            input,
            currentUser->id
        );

        // success
        return drogular::ActionResult::redirect(
            "/projects?success=project_created"
        );
    }
};