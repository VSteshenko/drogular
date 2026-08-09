# `RenderContext`

**Namespace:** `drogular`  
**Header:** `<drogular/render_context.hpp>`  
**Kind:** Class

## Purpose

`RenderContext` is the rendering environment for a single page request or a nested rendering scope.

It stores typed values used during rendering and provides access to application services, the current HTTP request, route parameters, localization, and GraphQL execution/result data.

Pages and components should keep request-specific rendering state in `RenderContext` rather than in long-lived object members.

---

## Role in Drogular

A root `RenderContext` is created by the router for each matching page request.

```text
HTTP GET request
      │
      ▼
    Router
      │
      ├── ApplicationServices
      ├── HttpRequest
      └── route parameters
      │
      ▼
RenderContext (root)
      │
      ├── typed values
      ├── service resolution
      ├── localization
      └── GraphQL result data
      │
      ▼
     Page
      │
      ▼
Template / Component rendering
      │
      └── child RenderContexts
```

The router passes the same root context to `Page::onInit()` and `Page::render()`.

Nested templates and components may create child contexts. A child can read values from its parent, but locally stored values remain isolated from the parent.

---

## Construction

### Default context

```cpp
RenderContext();
```

Creates an empty root context with no services, request, GraphQL client, route parameters, or parent.

Most application code receives an already configured context from Drogular rather than constructing one directly.

### Parent context

```cpp
explicit RenderContext(const RenderContext* parent);
```

Creates a context that falls back to `parent` when resolving typed values.

Prefer `createChild()` when creating nested rendering scopes because it also copies the current service-container and GraphQL-client pointers.

---

## Child Contexts

### `createChild()`

```cpp
RenderContext createChild() const;
```

Creates a child context whose parent is the current context.

The child:

- reads missing typed values from the parent;
- receives the same `ApplicationServices*` pointer;
- receives the same directly assigned `GraphQLClient*` pointer;
- starts with its own local value map;
- starts with its own scoped-service cache;
- starts with an empty [`GraphQLResult`](graphql-result.md);
- does **not** copy the current `HttpRequestPtr`;
- does **not** copy route parameters.

Template loops and component rendering use child contexts to introduce local values without overwriting parent values.

```cpp
auto child = context.createChild();
child.set("item", std::string("First"));
```

### Parent lookup

`get<T>()` and `contains()` search the local context first and then follow the parent chain.

```cpp
context.set("title", std::string("Users"));

auto child = context.createChild();

const auto title = child.get<std::string>("title");
```

`remove()` and `clear()` affect only the local value map. Removing a local value may therefore reveal a value with the same key from a parent context.

---

## Template Values

### `set()`

```cpp
template <typename T>
void set(std::string key, T value);
```

Stores a typed value in the local context.

Existing local values with the same key are replaced.

```cpp
context.set("title", std::string("Users"));
context.set("hasUsers", true);
```

Values are stored as `std::any`. Retrieval requires the same stored C++ type.

### `setJson()`

```cpp
template <typename T>
void setJson(
    std::string key,
    const T& value
);
```

Converts a value through Drogular's `toJsonValue()` conversion support and stores the resulting `Json::Value`.

```cpp
context.setJson(
    "pagination",
    drogular::makePaginationModel(
        result.page,
        result.totalPages,
        pageUrl
    )
);
```

`TodoPWA` uses this pattern when exposing its pagination model to the template. See [`PaginationModel`](../pagination-and-request-parameters/pagination-model.md) for the navigation model and URL-builder behavior.

### `get()`

```cpp
template <typename T>
std::optional<T> get(
    const std::string& key
) const;
```

Returns the typed value when the key exists and the stored type matches.

If the key is not present locally, parent contexts are searched recursively.

Returns `std::nullopt` when:

- the key cannot be found in the context chain; or
- a value exists but has a different C++ type.

### `require()`

```cpp
template <typename T>
T require(
    const std::string& key
) const;
```

Returns a typed value or throws `RenderContextError` when it is missing or has a different type.

```cpp
const auto title =
    context.require<std::string>("title");
```

### `getOr()`

```cpp
template <typename T>
T getOr(
    const std::string& key,
    T defaultValue
) const;
```

Returns the typed value or the supplied default when lookup fails.

### `contains()`

```cpp
bool contains(
    const std::string& key
) const;
```

Returns `true` when the key exists locally or in any parent context.

This checks only key presence. It does not verify a stored type.

### `remove()`

```cpp
void remove(
    const std::string& key
);
```

Removes the local value for `key`.

Parent values are not modified.

### `clear()`

```cpp
void clear();
```

Clears only locally stored typed values.

It does not clear:

- parent values;
- services;
- the scoped-service cache;
- GraphQL result data;
- the request;
- route parameters.

---

## Application Services

### `setServices()`

```cpp
void setServices(
    ApplicationServices* services
);
```

Assigns the application service container.

The router does this before a page receives the context.

### `services()`

```cpp
ApplicationServices* services();
const ApplicationServices* services() const;
```

Returns the assigned service container or `nullptr`.

### `hasServices()`

```cpp
bool hasServices() const;
```

Returns whether an `ApplicationServices` pointer is assigned.

### `service<T>()`

```cpp
template <typename T>
std::shared_ptr<T> service();
```

Resolves an application service.

Resolution first checks the context-local scoped-service cache. If no cached scoped service exists, the context asks `ApplicationServices` to create a scoped instance. When no scoped registration exists, normal application service resolution is used.

```cpp
const auto store =
    context.service<TodoStore>();
```

For a scoped registration, repeated calls for the same type on the **same context** return the same instance.

Each child context has its own scoped-service cache, so a scoped service resolved from a child may be a different instance from the one resolved by its parent.

Returns `nullptr` when no matching service can be resolved.

### `requireService<T>()`

```cpp
template <typename T>
std::shared_ptr<T> requireService();
```

The non-const overload uses `service<T>()` and therefore participates in `RenderContext` scoped-service caching.

It throws `std::runtime_error` when the service cannot be resolved.

```cpp
auto users =
    context.requireService<PortalUserProvider>();
```

A const overload also exists:

```cpp
template <typename T>
std::shared_ptr<T> requireService() const;
```

The const overload delegates directly to `ApplicationServices::requireService<T>()`. It does not use the context-local scoped-service cache and does not create scoped services through `RenderContext::service<T>()`.

---

## HTTP Request

### `setRequest()`

```cpp
void setRequest(
    const drogon::HttpRequestPtr& request
);
```

Assigns the current Drogon request.

The router sets it on the root context before calling `Page::onInit()`.

### `request()`

```cpp
drogon::HttpRequestPtr request() const;
```

Returns the assigned request pointer.

```cpp
const auto request = context.request();

const auto search = request != nullptr
    ? request->getParameter("search")
    : std::string();
```

A child created with `createChild()` does not copy the parent's request pointer.

### `cookie()`

```cpp
std::optional<std::string> cookie(
    const std::string& name
) const;
```

Returns a non-empty cookie value from the current request.

Returns `std::nullopt` when:

- no request is assigned;
- the cookie is absent; or
- Drogon returns an empty cookie value.

Localization uses this method to resolve the current `lang` cookie.

---

## Route Parameters

### `setRouteParam()`

```cpp
void setRouteParam(
    const std::string& name,
    const std::string& value
);
```

Stores a route parameter in the current context.

This method is intended primarily for framework use. The router populates route parameters after matching a parameterized page route.

### `routeParam()`

```cpp
std::optional<std::string> routeParam(
    const std::string& name
) const;
```

Returns a route parameter or `std::nullopt` when it is missing.

```cpp
const auto id = context.routeParam("id");
```

### `requireRouteParam()`

```cpp
std::string requireRouteParam(
    const std::string& name
) const;
```

Returns the route parameter or throws `std::runtime_error` with a `Missing route parameter` message.

Route parameters are local to a context. `createChild()` does not copy them and route-parameter lookup does not consult the parent context.

---

## Localization

### `translate()`

```cpp
std::string translate(
    const std::string& key
);
```

Translates a key through `TranslationSupport` using the current request locale.

```cpp
context.set(
    "usersRole",
    context.translate("users.field.role")
);
```

When no `TranslationProvider` is registered, the translation infrastructure returns the key itself.

See [`TranslationProvider`](../localization/translation-provider.md), [`TranslationSupport`](../localization/translation-support.md), and [`LocaleSupport`](../localization/locale-support.md).

### `setTranslated()`

```cpp
void setTranslated(
    const std::string& name,
    const std::string& translationKey
);
```

Translates `translationKey` and stores the resulting `std::string` under `name`.

### `setTranslations()`

```cpp
void setTranslations(
    std::initializer_list<
        std::pair<std::string, std::string>
    > values
);
```

Translates and stores multiple `(context name, translation key)` pairs.

```cpp
context.setTranslations({
    {"navUsers", "nav.users"},
    {"navDepartments", "nav.departments"},
    {"commonPrevious", "common.previous"}
});
```

---

## GraphQL

`RenderContext` can execute a GraphQL query or mutation and merge the returned fields into its internal [`GraphQLResult`](graphql-result.md).

### `setGraphQLClient()`

```cpp
void setGraphQLClient(
    GraphQLClient* client
);
```

Assigns a non-owning GraphQL client pointer directly to the context.

### `hasGraphQLClient()`

```cpp
bool hasGraphQLClient() const;
```

Returns whether a direct client pointer was assigned.

This does **not** report a GraphQL client available only through `ApplicationServices`.

### `executeGraphQL()` — query

```cpp
void executeGraphQL(
    const gql::Query& query,
    const GraphQLVariables& variables = {}
);
```

### `executeGraphQL()` — mutation

```cpp
void executeGraphQL(
    const gql::Mutation& mutation,
    const GraphQLVariables& variables = {}
);
```

Execution chooses a client in this order:

1. `ApplicationServices::graphQLClient()` when services are assigned and contain a GraphQL client;
2. the pointer assigned by `setGraphQLClient()`;
3. otherwise `RenderContextError("GraphQL client is not set")` is thrown.

The response is converted to [`GraphQLResult`](graphql-result.md) and merged into existing GraphQL result data. Existing fields with matching names are replaced; unrelated fields remain.

```cpp
context.executeGraphQL(query);

const auto viewer =
    context.graphql()
        .require<Json::Value>("viewer");
```

### `graphql()`

```cpp
GraphQLResult& graphql();
const GraphQLResult& graphql() const;
```

Returns the context-local GraphQL result store.

### `mergeGraphQL()`

```cpp
void mergeGraphQL(
    GraphQLResult result
);
```

Merges fields into the current GraphQL result. Incoming values replace existing values with the same key.

Child contexts start with an empty [`GraphQLResult`](graphql-result.md); GraphQL data is not inherited from the parent.

---

## Errors

### `RenderContextError`

`require<T>()` throws `RenderContextError` when a value is missing or has the wrong type.

GraphQL execution also throws `RenderContextError` when no client is available.

`requireRouteParam()` and `requireService<T>()` also throw `RenderContextError` when the required route parameter or service cannot be resolved.

---

## Lifetime and Thread Safety

A root `RenderContext` is created per page request by the current router implementation and destroyed after the request handler finishes.

Nested rendering creates additional child contexts with shorter lifetimes.

A context is mutable and has no internal synchronization. It should remain inside its rendering/request flow and should not be shared concurrently between threads without external synchronization.

Pointers stored by the context are non-owning:

- `ApplicationServices*`;
- directly assigned `GraphQLClient*`;
- `const RenderContext* parent`.

The pointed-to objects must outlive any context that uses them.

---

## Example

The TodoPWA page uses `RenderContext` to resolve a store, inspect the current request, prepare typed template values, convert a pagination model to JSON, and add PWA helpers:

```cpp
void TodoPage::onInit(
    drogular::RenderContext& context
) {
    const auto store =
        context.service<TodoStore>();

    auto query =
        TodoQueryParser::fromRequest(
            context.request()
        );

    const auto result =
        store->find(query);

    context.set(
        "title",
        std::string("Drogular Todo PWA")
    );
    context.set("search", query.search);
    context.set("hasTodos", !result.items.empty());
    context.set("totalItems", result.totalItems);

    context.setJson(
        "pagination",
        drogular::makePaginationModel(
            result.page,
            result.totalPages,
            pageUrl
        )
    );

    drogular::PageSupport::apply(
        context,
        "Drogular Todo PWA"
    );
}
```

PortalDemo additionally uses the same context for localization and required service resolution:

```cpp
context.set(
    "usersRole",
    context.translate("users.field.role")
);

auto users =
    context.requireService<PortalUserProvider>();
```

---

## Related Types

- [`AuthSupport`](../authentication-and-sessions/auth-support.md)
- [`SessionStore`](../authentication-and-sessions/session-store.md)
- [`Page`](../pages/page.md)
- [`TemplatePage`](../pages/template-page.md)
- [`Component`](../components/component.md)
- [`TranslationProvider`](../localization/translation-provider.md)
- [`ApplicationServices`](../dependency-injection/application-services.md)
- [`GraphQLResult`](graphql-result.md)
- [`GraphQLClient`](../graphql/client/graphql-client.md)
- [`RenderResult`](../testing/render-result.md)

---

## See Also

### Getting Started

- [Routing](../../getting-started/routing.md)
- [Components](../../getting-started/components.md)
- [Dependency Injection](../../getting-started/dependency-injection.md)

### Cookbook

- [Localization](../../cookbook/localization.md)
- [GraphQL](../../cookbook/graphql.md)
- [State Management](../../cookbook/state-management.md)
