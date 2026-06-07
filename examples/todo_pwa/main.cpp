#include <drogular/app.hpp>
#include <drogular/page.hpp>

#include <optional>
#include <string>
#include <vector>

struct Todo {
    int id;
    std::string title;
    bool done;
};

class TodoPage final : public drogular::Page {
public:
    void onInit(drogular::RenderContext&) override {
        todos_ = {
            {1, "Create Drogular project skeleton", true},
            {2, "Add GoogleTest support", true},
            {3, "Build GraphQL query builder", true},
            {4, "Create component/page framework", true},
            {5, "Wrap Drogon routing in App", true},
            {6, "Build Todo PWA example", false}
        };
    }

    std::optional<drogular::gql::Query> query() const override {
        return drogular::gql::query("TodoPage")
            .select("todos", {
                drogular::gql::field("id"),
                drogular::gql::field("title"),
                drogular::gql::field("done")
            });
    }

    std::string render(drogular::RenderContext&) override {
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

        for (const auto& todo : todos_) {
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

private:
    std::vector<Todo> todos_;
};

int main() {
    drogular::App app;

    app.page<TodoPage>("/");
    app.run(8080);
}
