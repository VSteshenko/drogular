#pragma once

#include "todo_service.hpp"

#include <drogular/action_handler.hpp>

class CreateTodoAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto title =
            context.form<std::string>("title")
                .value_or("");

        auto service =
            context.requireService<TodoService>();

        service->create(title);

        return drogular::ActionResult::redirect("/");
    }
};
