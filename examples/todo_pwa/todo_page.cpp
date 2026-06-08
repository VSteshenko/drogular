#include "todo_page.hpp"
#include "todo.hpp"

#include <vector>

void TodoPage::onInit(drogular::RenderContext& context) {
    const auto pageQuery = query();

    if (pageQuery.has_value()) {
        context.executeGraphQL(*pageQuery);
    }
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
