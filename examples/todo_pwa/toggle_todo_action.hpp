#pragma once

#include "todo_service.hpp"

#include <drogular/action_handler.hpp>

#include <cstdlib>

class ToggleTodoAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto idValue =
            context.formValue("id").value_or("");

        if (!idValue.empty()) {
            auto service =
                context.requireService<TodoService>();

            service->toggle(std::atoi(idValue.c_str()));
        }

        return drogular::ActionResult::redirect("/");
    }
};
