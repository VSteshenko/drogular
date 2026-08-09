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

Each request receives a fresh handler instance. The handler should still keep shared application state in services rather than treating action members as a persistence mechanism.

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

`App::action<ActionType>()` registers a factory that creates the handler with `std::make_shared<ActionType>()` for each matching request.

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

A new action instance is created for each matching POST request. Action instance members are therefore request-local and are not shared between concurrent requests.

```cpp
class CreateTodoAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto title =
            context.requireForm<std::string>("title");

        // Local variables and this action instance belong to this request.
        // ...

        return drogular::ActionResult::redirect("/");
    }
};
```

Singleton services, static data, repositories, and other objects referenced by the action may still be shared across requests and must provide their own thread-safety guarantees.

---

## Error Behavior

The action router catches exceptions raised by `handle()` and translates them to safe HTTP responses.

- [`ActionValidationError`](../forms-and-validation/action-validation-error.md) becomes `400 Bad Request` and exposes its validation message.
- Other `std::exception` values become `500 Internal Server Error` without exposing the exception message.
- Unknown exceptions also become `500 Internal Server Error`.

Expected input failures may still be handled explicitly through `ValidationResult` when the application wants to render field-level feedback or choose a custom response. Required accessors are appropriate when an invalid value should abort the action with the framework's standard `400` response.

See [Action Error Handling](error-handling.md).

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

- [`ActionAuthSupport`](../authentication-and-sessions/action-auth-support.md)
- [`ActionContext`](action-context.md)
- [`ActionResult`](action-result.md)
- [`App`](../application/app.md)
- [`Page`](../pages/page.md)
- [`FormValidator`](../forms-and-validation/form-validator.md)
