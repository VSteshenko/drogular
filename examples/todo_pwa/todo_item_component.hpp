#pragma once

#include <drogular/component.hpp>

class TodoItemComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "TodoItem";

    std::string templateHtml() const override {
        return R"(<li>{{ marker }} {{ title }}</li>)";
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
