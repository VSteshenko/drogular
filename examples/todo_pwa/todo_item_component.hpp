#pragma once

#include <drogular/component.hpp>

class TodoItemComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "TodoItem";

    std::string templateHtml() const override {
        return R"(
<li>
    <form method="post" action="/todos/toggle" style="display:inline">
        <input type="hidden" name="id" value="{{ id }}" />
        <button type="submit">{{ marker }}</button>
    </form>

    <span>{{ title }}</span>

    <form method="post" action="/todos/delete" style="display:inline">
        <input type="hidden" name="id" value="{{ id }}" />
        <button type="submit">Delete</button>
    </form>
</li>
)";
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
