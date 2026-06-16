#pragma once

#include "todo_service.hpp"

#include <drogular/action_handler.hpp>

class DeleteTodoAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        auto service =
            context.requireService<TodoService>();

        service->remove(
            context.requireForm<int>("id")
        );

        return drogular::ActionResult::redirect("/");
    }
};
