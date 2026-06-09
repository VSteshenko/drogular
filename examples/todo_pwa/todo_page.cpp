#include "todo_page.hpp"
#include "todo.hpp"

#include <vector>
#include <json/json.h>

void TodoPage::onInit(drogular::RenderContext& context) {
    const auto pageQuery = query();

    if (pageQuery.has_value()) {
        context.executeGraphQL(*pageQuery);
    }

    const auto sourceTodos =
        context.graphql()
            .require<std::vector<Todo>>("todos");

    Json::Value todos(Json::arrayValue);

    for (const auto& sourceTodo : sourceTodos) {
        Json::Value todo;

        todo["id"] = sourceTodo.id;
        todo["title"] = sourceTodo.title;
        todo["done"] = sourceTodo.done;

        todos.append(todo);
    }

    context.set("title", std::string("Drogular Todo PWA"));
    context.set(
        "subtitle",
        std::string("Angular-inspired C++ web framework for Drogon.")
    );
    context.set("todos", todos);
}

std::optional<drogular::gql::Query> TodoPage::query() const {
    return drogular::gql::query("TodoPage")
        .select(
            drogular::gql::field("todos")
                .children({
                    drogular::gql::field("id"),
                    drogular::gql::field("title"),
                    drogular::gql::field("done")
                })
        );
}

std::string TodoPage::templateHtml() const {
    return R"(
<!doctype html>
<html>
<head>
    <meta charset="utf-8">
    <title>{{ title }}</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
    <main>
        <h1>{{ title }}</h1>
        <p>{{ subtitle }}</p>

        <h2>Todo list</h2>
        <ul>
@foreach(todo in todos)
@if(todo.done)
            <li>[x] {{ todo.title }}</li>
@else
            <li>[ ] {{ todo.title }}</li>
@endif
@endforeach
        </ul>
    </main>
</body>
</html>
)";
}