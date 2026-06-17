#pragma once

#include "todo_store.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/form_validator.hpp>

class CreateTodoAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto validation =
            drogular::FormValidator(context)
                .required("title")
                .minLength("title", 2)
                .maxLength("title", 100)
                .validate();

        if (!validation.valid()) {
            return drogular::ActionResult::redirect("/");
        }

        auto store =
            context.requireService<TodoStore>();

        store->create(
            context.requireForm<std::string>("title")
        );

        return drogular::ActionResult::redirect("/");
    }
};
