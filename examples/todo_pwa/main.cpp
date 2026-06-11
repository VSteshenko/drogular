#include "todo.hpp"
#include "todo_page.hpp"
#include <todo_item_component.hpp>

#include <drogular/app.hpp>
#include <drogular/graphql_client.hpp>

#include <memory>
#include <vector>

int main() {
    const std::vector<Todo> todos = {
        {1, "Create Drogular project skeleton", true},
        {2, "Add GoogleTest support", true},
        {3, "Build GraphQL query builder", true},
        {4, "Create component/page framework", true},
        {5, "Wrap Drogon routing in App", true},
        {6, "Build Todo PWA example", true},
        {7, "Add RenderContext data store", true},
        {8, "Add fake GraphQL result", true},
        {9, "Render todos from GraphQL result", true},
        {10, "Add ApplicationServices", true},
        {11, "Load todos through GraphQLClient", false}
    };

    drogular::GraphQLResult result;
    result.set("todos", todos);

    auto client = std::make_shared<drogular::StaticGraphQLClient>(
        std::move(result)
    );

    drogular::App app;

    app.graphQLClient(client);
    app.component<TodoItemComponent>();
    app.page<TodoPage>("/");

    app.run(8080);
}