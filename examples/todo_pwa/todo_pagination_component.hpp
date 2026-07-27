#pragma once

#include <drogular/component.hpp>
#include <drogular/render_context.hpp>

#include <json/json.h>

class TodoPaginationComponent final : public drogular::TemplateComponent {
public:
    static constexpr auto tag = "TodoPagination";

    void onInit(drogular::RenderContext& context) override {
        const auto modelName = input<std::string>("model").value_or("");
        context.set(
            "paginationModel",
            context.getOr<Json::Value>(modelName, Json::Value())
        );
    }

    std::string templatePath() const override {
        return "components/todo_pagination.html";
    }
};