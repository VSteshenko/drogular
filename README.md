# Drogular

Drogular is an Angular-inspired C++ web framework built on top of Drogon.

The goal is to provide a component-oriented way to build SSR, SPA, and PWA-style applications in modern C++ with first-class GraphQL support.

### Why Drogular?

Drogular combines:

- Drogon performance
- Angular-inspired components
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
Page / Action
    ↓
Services
    ↓
Template Rendering
    ↓
HTML
```

## Status

**Version:** 0.14.0

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

Action pipeline:

```text
HTML Form
    ↓
ActionHandler
    ↓
ActionContext
    ↓
Services
    ↓
ActionResult
    ↓
HTTP Response
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
- Component lifecycle hooks
- Component initialization
- Component destruction
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

#### Actions

- ActionHandler
- ActionContext
- ActionResult
- ActionValidationError
- Form value access
- Typed form values
- Required form values
- Redirect responses
- HTML responses
- JSON responses
- Action integration tests

#### State Management

- State<T>
- Observable state
- State subscriptions
- Component state
- Shared state through DI
- Store pattern

#### Forms & Validation

- ValidationResult
- ValidationError
- FormValidator
- Required validation
- Minimum length validation
- Maximum length validation
- Email validation
- Field error helpers
- Form validation in actions

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
| Component Lifecycle             | Stable      |
| Lifecycle Hooks                 | Stable      |
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
| Actions                         | Stable      |
| ActionContext                   | Stable      |
| ActionResult                    | Stable      |
| Action Validation               | Stable      |
| Forms                           | Stable      |
| State<T>                        | Stable      |
| Observable State                | Stable      |
| Shared State                    | Stable      |
| Store Pattern                   | Stable      |
| Forms & Validation              | Stable      |
| ValidationResult                | Stable      |
| FormValidator                   | Stable      |
| Field Validation                | Stable      |
| Documentation                   | In Progress |
| Production Readiness            | In Progress |

## Example

TodoPWA demonstrates:

- Components
- Dependency Injection
- Template Compiler
- Forms
- Actions
- Validation

Template:

```html
<form method="post" action="/todos/create">
    <input name="title" />
    <button>Add</button>
</form>

@foreach(todo in todos)
    <TodoItem 
        id="{{ todo.id }}"
        title="{{ todo.title }}"
        done="{{ todo.done }}" />
@endforeach
```

Action:

```c++
class CreateTodoAction
    : public drogular::ActionHandler
{
public:
    ActionResult handle(
        ActionContext& context
    ) override
    {
        auto service =
            context.requireService<TodoService>();

        service->create(
            context.requireForm<std::string>("title")
        );

        return ActionResult::redirect("/");
    } 
};
```

Application setup:

```c++
drogular::App app;

app.services().add<TodoService>(
    drogular::ServiceLifetime::Singleton
);

app.component<TodoItemComponent>();

app.page<TodoPage>("/");
app.action<CreateTodoAction>("/todos/create");
app.action<ToggleTodoAction>("/todos/toggle");
app.action<DeleteTodoAction>("/todos/delete");

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

## Register a singleton

```c++
services.add<Logger>(
    drogular::ServiceLifetime::Singleton
);
```

## Register a scoped service

```c++
services.add<RequestContext>(
    drogular::ServiceLifetime::Scoped
);
```

## Register a transient service

```c++
services.add<CommandHandler>(
    drogular::ServiceLifetime::Transient
);
```

## Register with a factory

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

## Resolve from RenderContext

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

## Actions

### Register an action

```c++
app.action<CreateTodoAction>(
    "/todos/create"
);
```

### Read typed form values

```c++
auto title =
    context.form<std::string>("title");

auto id =
    context.form<int>("id");
```

### Required values

```c++
auto title =
    context.requireForm<std::string>("title");
```

### Resolve services

```c++
auto service =
    context.requireService<TodoService>();
```

### Redirect

```c++
return ActionResult::redirect("/");
```

### JSON response

```c++
Json::Value json;
json["ok"] = true;

return ActionResult::json(json);
```

## State Management

Drogular provides lightweight state management built around
plain C++ objects.

### State

```cpp
drogular::State<int> counter(0);

counter.set(42);

EXPECT_EQ(
    counter.value(),
    42
);
```

### Observable State

```c++
counter.subscribe(
    [](const int& value) {
        std::cout << value;
    }
);

counter.set(5);
```

### Shared State

```c++
using CounterState =
    drogular::State<int>;

services.addFactory<CounterState>(
    drogular::ServiceLifetime::Singleton,
    []() {
        return std::make_shared<CounterState>(0);
    }
);
```

### Store Pattern

```c++
class TodoStore {
public:
    drogular::State<
        std::vector<Todo>
    > todos;
};
```

Stores are regular services and integrate naturally with
dependency injection.

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

## Component Lifecycle

Components can participate in the rendering lifecycle.

```c++
class UserCardComponent
    : public drogular::TemplateComponent
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override
    {
        // initialization
    }

    void onDestroy(
        drogular::RenderContext& context
    ) override
    {
        // cleanup
    }
};
```

### Lifecycle order

```text
Component Creation
    ↓
onInit()
    ↓
render()
    ↓
onDestroy()
```

### Lifecycle hooks are invoked for

* Direct component rendering
* Component tags
* Nested components
* Sibling components

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

### 0.15 — Authentication & Authorization Sample App

- Login
- Logout
- Session handling
- Protected pages
- Role-based access example

### 1.0 — Stable Release

- API stabilization
- Service diagnostics
- Better template diagnostics
- Improved error messages
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