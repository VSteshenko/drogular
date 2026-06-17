#include <drogular/component.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/services.hpp>
#include <drogular/state.hpp>

#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <json/json.h>

struct StoreTodo {
    int id = 0;
    std::string title;
};

class TodoStore {
public:
    drogular::State<std::vector<StoreTodo>> todos{
        std::vector<StoreTodo>{
            {1, "Learn State"},
            {2, "Build Store"}
        }
    };
};

class StoreTodoListComponent final
    : public drogular::TemplateComponent
{
public:
    static constexpr auto tag = "StoreTodoList";

    void onInit(
        drogular::RenderContext& context
    ) override {
        auto store =
            context.requireService<TodoStore>();

        Json::Value todos(Json::arrayValue);

        for (const auto& sourceTodo :
             store->todos.value()) {
            Json::Value todo;
            todo["id"] = sourceTodo.id;
            todo["title"] = sourceTodo.title;

            todos.append(todo);
        }

        context.set("todos", todos);
    }

    std::string templateHtml() const override {
        return R"(
<ul>
@foreach(todo in todos)
<li>{{ todo.title }}</li>
@endforeach
</ul>
)";
    }
};

TEST(StorePatternTests, ComponentCanRenderApplicationStore) {
    drogular::ApplicationServices services;

    services.add<TodoStore>(
        drogular::ServiceLifetime::Singleton
    );

    services.components()
        .registerComponent<StoreTodoListComponent>();

    drogular::RenderContext context;
    context.setServices(&services);

    const auto html =
        drogular::component_renderer::render(
            "<StoreTodoList />",
            services.components(),
            context
        );

    EXPECT_TRUE(
        html.find("Learn State") != std::string::npos
    );

    EXPECT_TRUE(
        html.find("Build Store") != std::string::npos
    );
}