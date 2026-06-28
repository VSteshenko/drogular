#pragma once

#include <drogular/render_context.hpp>

class TodoItemComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "TodoItem";

    std::string templatePath() const override {
        return "components/todo_item.html";
    }

    void onInit(drogular::RenderContext& context) override {
        const auto done =
            input<std::string>("done")
                .value_or("false");

        context.set(
            "marker",
            done == "true" ? std::string("[x]") : std::string("[ ]")
        );
    }
};