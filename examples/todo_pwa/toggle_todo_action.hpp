#pragma once

#include "todo_service.hpp"

#include <drogular/action_handler.hpp>

class ToggleTodoAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        auto service =
            context.requireService<TodoService>();

        service->toggle(
            context.requireForm<int>("id")
        );

        return drogular::ActionResult::redirect("/");
    }
};