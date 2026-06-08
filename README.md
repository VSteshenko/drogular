# Drogular

Drogular is an Angular-inspired C++ web framework built on top of Drogon.

The goal is to provide a component-oriented way to build SSR, SPA, and PWA-style applications in modern C++ with first-class GraphQL support.

## Why Drogular?

Drogular brings together:

- Drogon performance
- Angular-inspired components
- GraphQL-first data loading
- Server-side rendering
- Testable pages and components
- Modern C++20

Example architecture:

```text
Application
    ↓
ApplicationServices
    ↓
Router
    ↓
RenderContext
    ↓
Page
    ↓
GraphQLClient
    ↓
GraphQLResult
```

## Status

Version: 0.3

Current features:

- CMake project skeleton
- Drogon integration
- Page and component base classes
- Simple routing through `drogular::App`
- GraphQL query builder
    - Nested selections
    - Typed values
    - Arguments
    - Variables
    - Aliases
    - Fragments
    - Basic validation
- GoogleTest-based test infrastructure
- Todo PWA example
- GitHub Actions CI for Ubuntu and macOS
- GraphQL client abstraction
- Static GraphQL client for tests and examples
- HTTP GraphQL client MVP
- RenderContext GraphQL result merging
- ApplicationServices foundation for future DI
- Component tree
- Slot support via `<slot/>`

## GraphQL builder

Drogular includes a small GraphQL query builder.

```cpp
const auto userFields = drogular::gql::fragment("UserFields", "User", {
    drogular::gql::field("id"),
    drogular::gql::field("name")
});

const auto query = drogular::gql::query("UserPage")
    .variable("userId", "ID!")
    .select(
        drogular::gql::field("user")
            .alias("profile")
            .arg("id", drogular::gql::variable("userId"))
            .children({
                drogular::gql::spread("UserFields")
            })
    )
    .fragment(userFields);

const auto validation = query.validate();

if (validation.valid()) {
    const auto text = query.toString();
}

```

## GraphQL clients

Drogular provides a small GraphQL client abstraction.

```cpp
drogular::GraphQLResult result;
result.set("todos", todos);

auto client = std::make_shared<drogular::StaticGraphQLClient>(
    std::move(result)
);

drogular::App app;

app.graphQLClient(client);
app.page<TodoPage>("/");
app.run(8080);

```

## Example

A page can declare a GraphQL query, load data through the configured GraphQL client, and render HTML using the render context.

```cpp
#include <drogular/app.hpp>
#include <drogular/graphql_client.hpp>
#include <drogular/page.hpp>

struct Todo {
    int id;
    std::string title;
    bool done;
};

class TodoPage final : public drogular::Page {
public:
    void onInit(drogular::RenderContext& context) override {
        const auto pageQuery = query();

        if (pageQuery.has_value()) {
            context.executeGraphQL(*pageQuery);
        }
    }

    std::optional<drogular::gql::Query> query() const override {
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

    std::string render(drogular::RenderContext& context) override {
        const auto todos =
            context.graphql()
                .require<std::vector<Todo>>("todos");

        std::string html = "<h1>Todo List</h1><ul>";

        for (const auto& todo : todos) {
            html += "<li>";
            html += todo.title;
            html += "</li>";
        }

        html += "</ul>";

        return html;
    }
};

int main() {
    std::vector<Todo> todos = {
        {1, "Learn Drogular", true},
        {2, "Build a PWA", false}
    };

    drogular::GraphQLResult result;
    result.set("todos", todos);

    auto client =
        std::make_shared<drogular::StaticGraphQLClient>(
            std::move(result)
        );

    drogular::App app;

    app.graphQLClient(client);

    app.page<TodoPage>("/");

    app.run(8080);
}
```

Data flow:

```text
GraphQL Query
        ↓
GraphQL Client
        ↓
GraphQL Result
        ↓
RenderContext
        ↓
Page
        ↓
HTML
```

Run the Todo PWA example:

```bash
./build/examples/todo_pwa/todo_pwa
```

Open:

```text
http://localhost:8080
```