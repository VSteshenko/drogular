# `ActionContext`

**Namespace:** `drogular`  
**Header:** `<drogular/action_context.hpp>`  
**Kind:** Class

## Purpose

`ActionContext` provides request-scoped data and application services while an `ActionHandler` executes.

It exposes the current Drogon request, submitted form values, route parameters, cookies, sessions, and dependency-injected services.

---

## Construction

```cpp
ActionContext(
    drogon::HttpRequestPtr request,
    ApplicationServices* services
);
```

Applications normally do not construct this object directly. The router creates one for each action request.

Before invoking the handler, the router also copies route parameters matched from the action route into the context.

---

## Request

### `request()`

```cpp
const drogon::HttpRequestPtr& request() const;
```

Returns the underlying Drogon request.

---

## Application Services

### `services()`

```cpp
ApplicationServices* services();
const ApplicationServices* services() const;
```

Returns the attached application service container, or `nullptr` when the context was created without one.

### `service<T>()`

```cpp
template <typename T>
std::shared_ptr<T> service();
```

Returns the registered service, or `nullptr` when no service container is attached or resolution does not produce a service.

### `requireService<T>()`

```cpp
template <typename T>
std::shared_ptr<T> requireService();
```

Requires a registered service.

If no `ApplicationServices` instance is attached, it throws:

```text
ApplicationServices not set
```

Other service-resolution failures follow `ApplicationServices::requireService<T>()` behavior.

---

## Form Values

### `formValue()`

```cpp
std::optional<std::string> formValue(
    const std::string& name
) const;
```

Reads a form parameter from the Drogon request.

An absent parameter and a parameter whose value is an empty string both return `std::nullopt`.

### `requireFormValue()`

```cpp
std::string requireFormValue(
    const std::string& name
) const;
```

Returns a required non-empty string value.

Throws [`ActionValidationError`](../forms-and-validation/action-validation-error.md) when the value is absent or empty.

### `form<T>()`

```cpp
template <typename T>
std::optional<T> form(
    const std::string& name
) const;
```

Supported types are currently:

- `std::string`
- `int`
- `double`
- `bool`

Numeric conversion is strict: the complete input string must represent the requested value. A numeric prefix followed by additional characters, such as `42abc`, returns `std::nullopt`.

For `bool`, the accepted values are:

| Input | Result |
| --- | --- |
| `true`, `1`, `on` | `true` |
| `false`, `0`, `off` | `false` |

Any other present, non-empty value returns `std::nullopt`.

### `requireForm<T>()`

```cpp
template <typename T>
T requireForm(
    const std::string& name
) const;
```

Returns a required typed form value.

Throws `ActionValidationError` when `form<T>()` returns `std::nullopt`.

---

## Cookies

### `cookie()`

```cpp
std::optional<std::string> cookie(
    const std::string& name
) const;
```

Returns a request cookie value.

A missing cookie or an empty cookie value returns `std::nullopt`.

---

## Sessions

### `existingSession()`

```cpp
std::shared_ptr<Session> existingSession() const;
```

Looks up the current session using the `session_id` request cookie.

Returns `nullptr` when:

- no services object is attached;
- `SessionStore` is not registered;
- the request has no non-empty `session_id` cookie;
- the store has no matching session.

### `session()`

```cpp
std::shared_ptr<Session> session();
```

Returns the existing session when one can be resolved; otherwise creates a new session through the required `SessionStore`.

If no services object is attached, the method returns `nullptr`.

When services are attached but `SessionStore` is unavailable, service resolution throws.

Creating a session does **not** automatically add a response cookie. The action must return the session id explicitly, for example:

```cpp
auto session = context.session();
session->set("username", username);

const auto sessionId =
    session->get("_id").value();

return drogular::ActionResult::redirect(
    "/dashboard"
).cookie(
    "session_id",
    sessionId
);
```

---

## Route Parameters

### `setRouteParam()`

```cpp
void setRouteParam(
    const std::string& name,
    const std::string& value
);
```

Stores a route parameter.

This method is intended primarily for framework use while matching a parameterized action route.

### `routeParam()`

```cpp
std::optional<std::string> routeParam(
    const std::string& name
) const;
```

Returns a matched route parameter or `std::nullopt`.

### `requireRouteParam()`

```cpp
std::string requireRouteParam(
    const std::string& name
) const;
```

Returns a required route parameter.

Throws `ActionContextError` when the parameter is missing.

---

## Lifetime and Thread Safety

The router creates one `ActionContext` for one incoming action request.

The context is not shared across requests. It does not provide internal synchronization and should remain within the current action flow.

The thread-safety of services returned through the context depends on the service implementation and configured service lifetime.

---

## Example

```cpp
drogular::ActionResult handle(
    drogular::ActionContext& context
) override {
    const auto id =
        context.requireRouteParam("id");

    const auto title =
        context.requireForm<std::string>("title");

    auto repository =
        context.requireService<ProjectRepository>();

    repository->update(id, title);

    return drogular::ActionResult::redirect(
        "/projects/" + id
    );
}
```

---

## Related Types

- [`Session`](../authentication-and-sessions/session.md)
- [`SessionStore`](../authentication-and-sessions/session-store.md)
- [`ActionAuthSupport`](../authentication-and-sessions/action-auth-support.md)
- [`ActionHandler`](action-handler.md)
- [`ActionResult`](action-result.md)
- [`ActionValidationError`](../forms-and-validation/action-validation-error.md)
- [`ApplicationServices`](../dependency-injection/application-services.md)
- [`Session`](../authentication-and-sessions/session.md)
- [`SessionStore`](../authentication-and-sessions/session-store.md)
