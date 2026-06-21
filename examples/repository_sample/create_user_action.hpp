#pragma once

#include "user_repository.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>

class RepositorySampleCreateUserAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override
    {
        const auto validation =
            drogular::FormValidator(context)
                .required("name")
                .minLength("name", 2)
                .required("email")
                .email("email")
                .validate();

        if (!validation.valid()) {
            return drogular::ActionResult::redirect(
                "/users"
            );
        }

        auto repository =
            context.requireService<
                RepositorySampleUserRepository
            >();

        repository->create(
            context.requireForm<std::string>("name"),
            context.requireForm<std::string>("email")
        );

        return drogular::ActionResult::redirect(
            "/users"
        );
    }
};