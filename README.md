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

**Version:** 0.5.0

Drogular is an experimental Angular-inspired C++ web framework built on top of Drogon.

Current architecture:

```text
Application
    ↓
ApplicationServices
    ↓
Service Container
    ↓
Router
    ↓
RenderContext
    ↓
Scoped Component Contexts
    ↓
TemplatePage / TemplateComponent
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
- HTML escaping
- Conditions (`@if`)
- If / Else (`@else`)
- Loops (`@foreach`)
- Component parameters
- Json object access: {{ user.name }} 
- Json conditions: @if(user.active)
- Json loops: @foreach(todo in todos)

### Testing

- Page rendering helpers
- Component rendering helpers
- HTML assertions
- Unit tests
- GitHub Actions CI

## Project Maturity

| Area | Status |
|--------|--------|
| Application Bootstrap | Stable |
| Routing | Stable |
| Components | Stable |
| Component Tree | Stable |
| Named Slots | Stable |
| Component Inputs | Stable |
| Scoped RenderContext | Stable |
| Template Engine | Stable |
| Variables (`{{ }}`) | Stable |
| Raw Output (`{{{ }}}`) | Stable |
| Conditions (`@if`) | Stable |
| Loops (`@foreach`) | Stable |
| Json Object Access | Stable |
| GraphQL Query Builder | Stable |
| GraphQL Validation | Stable |
| GraphQLResult | Stable |
| StaticGraphQLClient | Stable |
| HttpGraphQLClient | Experimental |
| Service Container | Experimental |
| Dependency Injection | Planned |
| Component Tags | Planned |
| Template Compilation | Planned |
## Example

A page can declare a GraphQL query, load data through the configured GraphQL client, and render HTML using Drogular templates.

```cpp
class TodoPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override {
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
        context.set("todos", todos);
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

    std::string templateHtml() const override {
        return R"(
<h1>{{ title }}</h1>

<ul>
@foreach(todo in todos)
@if(todo.done)
    <li>[x] {{ todo.title }}</li>
@else
    <li>[ ] {{ todo.title }}</li>
@endif
@endforeach
</ul>
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
Json data
        ↓
RenderContext
        ↓
TemplatePage
        ↓
@if / @else / @foreach / {{ todo.title }}
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

### 0.6 — Dependency Injection

- Constructor injection
- Service lifetimes
- Scoped services
- Service factories
- Typed service registration helpers
- Dependency validation

Example:

```cpp
class TodoPage : public drogular::TemplatePage {
public:
    explicit TodoPage(
        std::shared_ptr<TodoService> service
    );
};
```

### 0.7 — Component System

- Component tags
- Component registry
- Template component composition
- Dynamic component rendering
- Component discovery

Example:

```html
<Card title="Welcome"/>

<UserProfile user="{{ user }}"/>
```

### 0.8 — Template Compiler

- Template AST
- Compiled templates
- Faster rendering
- Template diagnostics
- Better error reporting

### Future

- GraphQL code generation
- GraphQL subscriptions
- WebSocket support
- Server-side hydration
- PWA tooling
- CLI
- Hot reload
- Static site generation