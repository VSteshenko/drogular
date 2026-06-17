#pragma once

#include "todo_store.hpp"

#include <drogular/action_handler.hpp>

class ToggleTodoAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        auto store =
            context.requireService<TodoStore>();

        store->toggle(
            context.requireForm<int>("id")
        );

        return drogular::ActionResult::redirect("/");
    }
};