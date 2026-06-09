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

**Version:** 0.4.0

Drogular is an experimental Angular-inspired C++ web framework built on top of Drogon.

Current architecture:

```text
Application
    ↓
ApplicationServices
    ↓
Router
    ↓
RenderContext
    ↓
Page / Component
    ↓
Template Engine
    ↓
HTML
```

Data pipeline:

```text
GraphQL Query
        ↓
GraphQL Client
        ↓
GraphQL Result
        ↓
RenderContext
        ↓
Template Rendering
        ↓
HTML
```

Current capabilities:

### Application Layer

- Application bootstrap
- Routing
- Application services
- Component tree
- Named slots

### GraphQL Layer

- GraphQL query builder
- Variables
- Aliases
- Fragments
- Validation
- StaticGraphQLClient
- HttpGraphQLClient (MVP)
- GraphQLResult
- Result merging

### Template Layer

- TemplatePage
- TemplateComponent
- HTML escaping
- Raw HTML output
- Variables (`{{ variable }}`)
- Conditions (`@if`)
- If / Else (`@else`)
- Loops (`@foreach`)
- Component parameters

### Testing

- Page rendering helpers
- Component rendering helpers
- HTML assertions
- Unit tests
- GitHub Actions CI

Project maturity:

| Area | Status |
|--------|--------|
| Routing | Stable |
| Components | Stable |
| Templates | Stable |
| GraphQL Builder | Stable |
| GraphQL Clients | Experimental |
| DI Container | Planned |
| Object Templates | Planned |
| Component Inputs | Planned |

## Current Limitations

The template engine is intentionally minimal in version 0.4.

Currently supported:

- `{{ variable }}`
- `{{{ raw }}}`
- `@if`
- `@else`
- `@foreach(item in items)`

Current loop support is limited to:

```cpp
std::vector<std::string>
```

Object access is planned for version 0.5:

```html
{{ user.name }}
{{ todo.title }}
```

## Example

A page can declare a GraphQL query, load data through the configured GraphQL client, and render HTML using Drogular templates.

```cpp
#include <drogular/app.hpp>
#include <drogular/graphql_client.hpp>
#include <drogular/page.hpp>

class TodoPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
        const auto pageQuery = query();

        if (pageQuery.has_value()) {
            context.executeGraphQL(*pageQuery);
        }

        context.set(
            "title",
            std::string("Drogular Todo PWA")
        );

        context.set(
            "showTodos",
            true
        );

        context.set(
            "items",
            std::vector<std::string>{
                "Learn Drogular",
                "Build a PWA"
            }
        );
    }

    std::optional<drogular::gql::Query> query() const override {
        return drogular::gql::query("TodoPage")
            .select(
                drogular::gql::field("todos")
                    .children({
                        drogular::gql::field("id"),
                        drogular::gql::field("title")
                    })
            );
    }

    std::string templateHtml() const override {
        return R"(
<h1>{{ title }}</h1>

@if(showTodos)
<ul>
@foreach(item in items)
    <li>{{ item }}</li>
@endforeach
</ul>
@endif
)";
    }
};
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
TemplatePage
        ↓
@if
@foreach
{{ variable }}
        ↓
HTML
```

Application setup:

```cpp
drogular::GraphQLResult result;

auto client =
    std::make_shared<
        drogular::StaticGraphQLClient
    >(std::move(result));

drogular::App app;

app.graphQLClient(client);

app.page<TodoPage>("/");

app.run(8080);
```

Run the Todo PWA example:

```bash
./build/examples/todo_pwa/todo_pwa
```

Open:

```text
http://localhost:8080
```

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

## Template Engine

### Variables

```html
<h1>{{ title }}</h1>
```

### Raw Output

```html
{{{ content }}}
```

### Conditions

```html
@if(showTitle)
<h1>{{ title }}</h1>
@endif
```

### If / Else

```html
@if(isLoggedIn)
<p>Welcome</p>
@else
<p>Please log in</p>
@endif
```

### Loops

```html
<ul>
@foreach(item in items)
<li>{{ item }}</li>
@endforeach
</ul>
```
## Layouts

```html
<header>
    <slot name="header"/>
</header>

<main>
    <slot name="content"/>
</main>
```
```cpp
class HeaderComponent : public drogular::TemplateComponent {
public:
    std::string slot() const override {
        return "header";
    }
};
```

## Roadmap

```text
0.5

- Scoped RenderContext
- Object access
- Component inputs
- Service container
- Dependency injection
```