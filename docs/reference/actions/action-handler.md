# `ActionHandler`

**Namespace:** `drogular`  
**Header:** `<drogular/action_handler.hpp>`  
**Kind:** Abstract class

## Purpose

`ActionHandler` is the base class for application commands mounted to POST routes.

A concrete action implements `handle(ActionContext&)`, performs its application work, and returns an `ActionResult` describing the HTTP response.

---

## Role in Drogular

`ActionHandler` is the command-side counterpart to `Page`.

Pages render GET requests. Actions process POST requests and normally modify application state, call repositories or services, manage sessions, and then redirect or return another response type.

```text
                 HTTP Request
                      │
             ┌────────┴────────┐
             │                 │
           GET               POST
             │                 │
             ▼                 ▼
           Page          ActionHandler
             │                 │
             ▼                 ▼
       RenderContext      ActionContext
             │                 │
             ▼                 ▼
           HTML           ActionResult
                               │
                               ▼
                         HTTP Response
```

The handler itself should contain behavior, not request-specific mutable state.

---

## Synopsis

```cpp
class ActionHandler {
public:
    virtual ~ActionHandler() = default;

    virtual ActionResult handle(
        ActionContext& context
    ) = 0;
};
```

---

## Construction and Registration

Actions are normally registered through `App`:

```cpp
app.action<CreateTodoAction>("/todos/create");
```

`App::action<ActionType>()` creates the handler with `std::make_shared<ActionType>()` and registers it with the router.

`ActionType` must therefore:

- inherit from `drogular::ActionHandler`;
- be default-constructible.

The current router registers action routes for POST requests only.

---

## Public API

### `handle()`

```cpp
virtual ActionResult handle(
    ActionContext& context
) = 0;
```

Executes the application action for the current request.

Use the supplied [`ActionContext`](action-context.md) to read submitted values, route parameters, cookies, sessions, and registered services.

Return an [`ActionResult`](action-result.md) to describe the response.

---

## Lifetime and Thread Safety

An action is instantiated once when its route is registered.

The router captures the same `std::shared_ptr<ActionHandler>` and reuses that object for every request to the route. Drogon may invoke the handler concurrently.

Do not store request-specific mutable state in action members:

```cpp
class CreateTodoAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto title =
            context.requireForm<std::string>("title");

        // Request-specific state stays local.
        // ...

        return drogular::ActionResult::redirect("/");
    }
};
```

Any mutable state stored on the handler requires external synchronization.

---

## Error Behavior

The router currently calls `handle()` directly and does not translate action-specific exceptions into `ActionResult` values.

For example, [`ActionValidationError`](../forms-and-validation/action-validation-error.md) thrown by `ActionContext::requireForm<T>()` is not caught by the action router itself.

Expected input failures should normally be validated before required accessors are called.

---

## Example

TodoPWA validates the submitted title, resolves the store, changes application state, and redirects back to the page:

```cpp
class CreateTodoAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto validation =
            drogular::FormValidator(context)
                .required("title")
                .minLength("title", 2)
                .maxLength("title", 100)
                .validate();

        if (!validation.valid()) {
            return drogular::ActionResult::redirect("/");
        }

        auto store =
            context.requireService<TodoStore>();

        store->create(
            context.requireForm<std::string>("title")
        );

        return drogular::ActionResult::redirect("/");
    }
};
```

---

## Related Types

- [`ActionContext`](action-context.md)
- [`ActionResult`](action-result.md)
- [`App`](../application/app.md)
- [`Page`](../pages/page.md)
- [`FormValidator`](../forms-and-validation/form-validator.md)
