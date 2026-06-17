#pragma once

#include "todo_store.hpp"

#include <drogular/action_handler.hpp>

class CreateTodoAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto title =
            context.form<std::string>("title")
                .value_or("");

        auto store =
            context.requireService<TodoStore>();

        store->create(
            context.form<std::string>("title").value_or("")
        );

        return drogular::ActionResult::redirect("/");
    }
};
