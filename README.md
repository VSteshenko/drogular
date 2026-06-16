# Drogular

Drogular is an Angular-inspired C++ web framework built on top of Drogon.

The goal is to provide a component-oriented way to build SSR, SPA, and PWA-style applications in modern C++ with first-class GraphQL support.

### Why Drogular?

Drogular combines:

- Drogon performance
- Angular-inspired components
- GraphQL-first data loading
- Server-side rendering
- Scoped component contexts
- Template engine
- Service container
- Modern C++20
- Strong testing support

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

**Version:** 0.10.0

Drogular is an experimental Angular-inspired C++ web framework built on top of Drogon.

Current architecture:

```text
Application
    ↓
ApplicationServices
    ↓
Dependency Injection
    ↓
Router
    ↓
RenderContext
    ↓
Component Registry
    ↓
TemplatePage / TemplateComponent
    ↓
Template Cache
    ↓
Compiled Template
    ↓
Template Runtime
    ↓
Component Renderer
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

### Features

#### Application

- Application bootstrap
- Routing
- Pages
- Render contexts

#### Components

- Component registry
- Dynamic component creation
- Recursive component rendering
- Self-closing component tags
- Component children
- Default slots
- Named slots
- Component inputs
- Attribute bindings

#### Dependency Injection

- Singleton services
- Lazy singleton services
- Transient services
- Scoped services
- Constructor injection factories
- Dependency graph
- Dependency validation
- Circular dependency detection

#### Template Compiler

- Template tokenizer
- Template AST
- Template parser
- Compiled templates
- Template runtime
- Template cache
- Template diagnostics
- Variables (`{{ value }}`)
- Raw HTML (`{{{ value }}}`)
- Conditions (`@if`)
- Loops (`@foreach`)
- Json path access

#### GraphQL

- Query builder
- Variables
- Aliases
- Fragments
- Validation
- GraphQLRequest
- GraphQLResponse
- GraphQLResult
- StaticGraphQLClient
- HttpGraphQLClient
- Error handling
- Integration tests

#### Testing

- Component testing
- Page testing
- Integration testing
- GitHub Actions CI

## Project Maturity

| Area                            | Status      |
|---------------------------------|-------------|
| Application Bootstrap           | Stable      |
| Routing                         | Stable      |
| Components                      | Stable      |
| Component Registry              | Stable      |
| Component Tags                  | Stable      |
| Component Inputs                | Stable      |
| Attribute Bindings              | Stable      |
| Slots                           | Stable      |
| Recursive Component Rendering   | Stable      |
| Scoped RenderContext            | Stable      |
| Template Compiler               | Stable      |
| Template AST                    | Stable      |
| Template Parser                 | Stable      |
| Template Runtime                | Stable      |
| Template Cache                  | Stable      |
| Template Diagnostics            | Stable      |
| Variables ({{ }})               | Stable      |
| Raw Output ({{{ }}})            | Stable      |
| Conditions (@if)                | Stable      |
| Loops (@foreach)                | Stable      |
| Json Object Access              | Stable      |
| GraphQL Query Builder           | Stable      |
| GraphQL Validation              | Stable      |
| GraphQLResult                   | Stable      |
| StaticGraphQLClient             | Stable      |
| HttpGraphQLClient               | Stable      |
| Dependency Injection            | Stable      |
| Service Lifetimes               | Stable      |
| Constructor Injection Factories | Stable      |
| Dependency Graph                | Stable      |
| Dependency Validation           | Stable      |
| Circular Dependency Detection   | Stable      |
| Testing Helpers                 | Stable      |
| Documentation                   | In Progress |
| Production Readiness            | In Progress |

## Example

A page can declare a GraphQL query, load data through the configured GraphQL client, and render HTML using Drogular templates.

```html
<AppLayout>
    <Card title="Todos">
        @foreach(todo in todos)
            <TodoItem
                title="{{ todo.title }}"
                done="{{ todo.done }}" />
        @endforeach
    </Card>
</AppLayout>
```

```c++
app.component<AppLayout>();
app.component<CardComponent>();
app.component<TodoItemComponent>();

app.page<TodoPage>("/");
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
Component Renderer
        ↓
HTML
```

Application setup:

```c++
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

## Dependency Injection

Drogular includes a small dependency injection container through `ApplicationServices`.

### Register a singleton

```c++
services.add<Logger>(
    drogular::ServiceLifetime::Singleton
);
```

### Register a scoped service

```c++
services.add<RequestContext>(
    drogular::ServiceLifetime::Scoped
);
```

### Register a transient service

```c++
services.add<CommandHandler>(
    drogular::ServiceLifetime::Transient
);
```

### Register with a factory

```c++
services.addFactory<TodoService>(
    drogular::ServiceLifetime::Scoped,
    [&services]() {
        return std::make_shared<TodoService>(
            services.requireService<TodoRepository>()
        );
    }
);
```

### Resolve from RenderContext

```c++
auto logger =
    context.requireService<Logger>();
```

| Lifetime      | Behavior                             |
|---------------|--------------------------------------|
| Singleton     | Created once and reused              |
| LazySingleton | Created on first request and reused  |
| Transient     | New instance on every request        |
| Scoped        | One instance per RenderContext scope |

## GraphQL builder

Drogular includes a small GraphQL query builder.

```c++
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

## GraphQL Client

Drogular provides a small GraphQL client abstraction.

```c++
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

### GraphQLRequest

```c++
GraphQLRequest request(
    query.toString()
);

request
    .variable("id", "42")
    .variable("limit", 10);
```

### Execute Request

```c++
auto response =
    client.executeRequest(request);

if (response.hasErrors()) {
    for (const auto& message :
         response.errorMessages()) {
        std::cout << message << '\n';
    }
}
```

### GraphQLResponse

```c++
if (response.hasData()) {
    auto viewer =
        response.field("viewer");
}
```

## Template Compiler

Templates are compiled into an AST and cached for reuse.

### Variables

```html
<h1>{{ title }}</h1>
```

### Raw HTML

```html
{{{ html }}}
```

### Conditions

```html
@if(showTodos)
<p>Visible</p>
@else
<p>Hidden</p>
@endif
```

### Loops

```html
@foreach(todo in todos)
    <li>{{ todo.title }}</li>
@endforeach
```
### Json Paths

```html
{{ user.profile.name }}
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
```c++
class HeaderComponent : public drogular::TemplateComponent {
public:
    std::string slot() const override {
        return "header";
    }
};
```

## Roadmap

### 0.11 — Developer Experience & Validation

- Startup validation
- Service diagnostics
- Application validation
- Better template diagnostics
- Improved error messages
- Validation reports

### 1.0 — Stable Release

- API stabilization
- Documentation
- Examples
- Production readiness

### Future

- GraphQL code generation
- GraphQL subscriptions
- WebSocket support
- Server-side hydration
- PWA tooling
- CLI
- Hot reload
- Static site generation