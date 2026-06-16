#pragma once

#include "todo_service.hpp"

#include <drogular/action_handler.hpp>

class ToggleTodoAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto id = context.form<int>("id");

        if (id.has_value()) {
            auto service =
                context.requireService<TodoService>();

            service->toggle(*id);
        }

        return drogular::ActionResult::redirect("/");
    }
};
