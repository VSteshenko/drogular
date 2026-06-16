#include "todo_page.hpp"
#include "todo.hpp"
#include "todo_service.hpp"

#include <vector>
#include <json/json.h>

void TodoPage::onInit(drogular::RenderContext& context) {
    auto service =
        context.requireService<TodoService>();

    const auto& sourceTodos =
        service->todos();

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
    context.set("hasTodos", !sourceTodos.empty());
}

std::optional<drogular::gql::Query> TodoPage::query() const {
    return std::nullopt;
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

        <form method="post" action="/todos/create">
            <input name="title" placeholder="New todo" />
            <button type="submit">Add</button>
        </form>

        <h2>Todo list</h2>

@if(hasTodos)
        <ul>
@foreach(todo in todos)
            <TodoItem
                title="{{ todo.title }}"
                done="{{ todo.done }}" />
@endforeach
        </ul>
@else
        <p>No todos yet.</p>
@endif
    </main>
</body>
</html>
)";
}