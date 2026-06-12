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

**Version:** 0.7.0

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

Current capabilities:

### Application Layer

- Application bootstrap
- Routing
- Component registry
- Dynamic component creation
- Component metadata tags
- Self-closing component tags
- Component tags with children
- Recursive component rendering
- Default and named slots
- Component inputs
- Attribute bindings
- Scoped render contexts
- Service container
- Dependency validation
- Service lifetimes:
    - Singleton
    - Lazy singleton
    - Transient
    - Scoped
- Factory registration with lifetimes

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
|---|---|
| Application Bootstrap | Stable |
| Routing | Stable |
| Components | Stable |
| Component Registry | Stable |
| Component Tags | Stable |
| Component Inputs | Stable |
| Slots | Stable |
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
| Dependency Injection | Stable |
| Service Lifetimes | Stable |
| Constructor Injection | Planned |
| Circular Dependency Detection | Planned |
| Template Compilation | Planned |

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

```cpp
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

## Dependency Injection

Drogular includes a small dependency injection container through `ApplicationServices`.

### Register a singleton

```cpp
services.add<Logger>(
    drogular::ServiceLifetime::Singleton
);
```

### Register a scoped service

```cpp
services.add<RequestContext>(
    drogular::ServiceLifetime::Scoped
);
```

### Register a transient service

```cpp
services.add<CommandHandler>(
    drogular::ServiceLifetime::Transient
);
```

### Register with a factory

```cpp
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

```cpp
auto logger =
    context.requireService<Logger>();
```

| Lifetime | Behavior                             |
|---|--------------------------------------|
| Singleton | Created once and reused              |
| LazySingleton | Created on first request and reused  |
| Transient | New instance on every request        |
| Scoped | One instance per RenderContext scope |

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

### 0.8 — Constructor Injection

- Constructor injection
- Service dependency graph
- Dependency validation
- Circular dependency detection
- Service factories cleanup

### 0.9 — Template Compiler

- Template AST
- Compiled templates
- Faster rendering
- Template diagnostics
- Better error reporting

### 1.0 Stable Release

- API Stabilization
- Documentation
- Examples
- Production Readiness

### Future

- GraphQL code generation
- GraphQL subscriptions
- WebSocket support
- Server-side hydration
- PWA tooling
- CLI
- Hot reload
- Static site generation