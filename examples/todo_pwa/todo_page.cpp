#include "todo_page.hpp"
#include "todo.hpp"

#include <vector>

void TodoPage::onInit(drogular::RenderContext& context) {
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
        {10, "Test TodoPage rendering", false}
    };

    context.graphql().set("todos", todos);
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

std::string TodoPage::render(drogular::RenderContext& context) {
    const auto todos = context.graphql().require<std::vector<Todo>>("todos");

    std::string html = R"(
<!doctype html>
<html>
<head>
    <meta charset="utf-8">
    <title>Drogular Todo PWA</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
    <main>
        <h1>Drogular Todo PWA</h1>
        <p>Angular-inspired C++ web framework for Drogon.</p>

        <h2>Todo list</h2>
        <ul>
)";

    for (const auto& todo : todos) {
        html += "            <li>";
        html += todo.done ? "[x] " : "[ ] ";
        html += todo.title;
        html += "</li>\n";
    }

    html += R"(        </ul>
    </main>
</body>
</html>
)";

    return html;
}
