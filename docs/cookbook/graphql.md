# GraphQL

GraphQL is the recommended way to communicate with backend services in Drogular.

Rather than exposing GraphQL throughout the application, Drogular treats GraphQL as an implementation detail of an action.

A component expresses a user intention.

An action performs the GraphQL operation.

The application state is updated through a store.

---

# GraphQL Belongs to Actions

Components, pages, and stores should not execute GraphQL requests directly.

Instead, they delegate the operation to an action.

```text
Component
    │
    ▼
Action
    │
    ▼
GraphQL
    │
    ▼
Store
    │
    ▼
UI Update
```

This keeps the communication layer isolated from the user interface.

---

# Why?

A user does not intend to execute a GraphQL mutation.

A user intends to:

- create a todo
- delete a project
- update a profile
- sign in
- upload a file

GraphQL is only one possible implementation of those operations.

If the backend changes from GraphQL to REST, only the action changes.

The rest of the application remains unchanged.

---

# HttpGraphQLClient

Drogular provides `HttpGraphQLClient` for executing GraphQL operations.

The client is an infrastructure component.

It should be injected into actions.

```cpp
class CreateTodoAction
{
public:
    explicit CreateTodoAction(
        std::shared_ptr<HttpGraphQLClient> graphqlClient)
        : graphqlClient_(std::move(graphqlClient))
    {
    }

private:
    std::shared_ptr<HttpGraphQLClient> graphqlClient_;
};
```

Pages and components should not own a GraphQL client.

---

# Queries

Queries retrieve data.

A typical workflow looks like this:

```text
Action
    │
    ▼
GraphQL Query
    │
    ▼
Response
    │
    ▼
Store
```

The action converts the response into application state.

---

# Mutations

Mutations modify data.

For example:

```text
User Click
      │
      ▼
CreateTodoAction
      │
      ▼
CreateTodoMutation
      │
      ▼
TodoStore.add(todo)
```

The component does not manipulate the returned data directly.

It reacts to changes in the store.

---

# Example Query

```cpp
class LoadTodosAction
{
public:
    ActionResult execute()
    {
        const auto response =
            graphqlClient_->execute(
                GetTodosQuery);

        if (!response)
        {
            return ActionResult::failure(
                "Unable to load todos.");
        }

        todoStore_->setTodos(response.todos);

        return ActionResult::success();
    }

private:
    std::shared_ptr<HttpGraphQLClient> graphqlClient_;
    std::shared_ptr<TodoStore> todoStore_;
};
```

The action performs three responsibilities:

- executes the query
- handles failures
- updates the store

---

# Example Mutation

```cpp
class CreateTodoAction
{
public:
    ActionResult execute(
        const CreateTodoForm& form)
    {
        CreateTodoVariables variables{
            .title = form.title
        };

        const auto response =
            graphqlClient_->execute(
                CreateTodoMutation,
                variables);

        if (!response)
        {
            return ActionResult::failure(
                "Unable to create todo.");
        }

        todoStore_->add(response.todo);

        return ActionResult::success();
    }

private:
    std::shared_ptr<HttpGraphQLClient> graphqlClient_;
    std::shared_ptr<TodoStore> todoStore_;
};
```

The action owns both the GraphQL operation and the resulting state update.

---

# Variables

GraphQL variables should be represented by dedicated C++ types.

```cpp
struct CreateTodoVariables
{
    std::string title;
};
```

Using strongly typed variables improves readability and compile-time safety.

---

# Results

GraphQL responses should be converted into domain models before entering the store.

```text
GraphQL Response
        │
        ▼
Domain Model
        │
        ▼
Store
```

The rest of the application should not depend on GraphQL-specific response structures.

---

# Error Handling

GraphQL requests may fail for many reasons:

- network failures
- authentication errors
- validation errors
- server errors
- unexpected responses

Actions should translate these failures into meaningful application results.

```cpp
if (!response)
{
    return ActionResult::failure(
        "Unable to connect to the server.");
}
```

Components display the error.

They do not interpret GraphQL responses.

---

# Authentication

Authenticated requests should be configured inside the GraphQL client.

Actions should not manually attach authorization headers.

Authentication is infrastructure.

Business operations remain focused on application behavior.

---

# Stores

GraphQL responses should update stores.

For example:

```text
GraphQL Response
        │
        ▼
TodoStore
        │
        ▼
TodoList
Sidebar
Dashboard
```

Every observing component automatically receives the updated state.

---

# Keep Components Clean

Components should never execute GraphQL requests.

Avoid:

```cpp
auto response =
    graphqlClient->execute(...);
```

Instead:

```cpp
createTodoAction_->execute(form);
```

Components express user intent.

Actions perform the work.

---

# Dependency Injection

GraphQL clients should be injected.

```cpp
class LoadTodosAction
{
public:
    LoadTodosAction(
        std::shared_ptr<HttpGraphQLClient> client,
        std::shared_ptr<TodoStore> store)
        : client_(std::move(client)),
          store_(std::move(store))
    {
    }

private:
    std::shared_ptr<HttpGraphQLClient> client_;
    std::shared_ptr<TodoStore> store_;
};
```

Actions declare every dependency explicitly.

---

# Testing

Actions should be tested without requiring a running GraphQL server.

Replace the client with a fake or mock implementation.

Typical tests include:

- successful query
- successful mutation
- network failure
- authentication failure
- GraphQL errors
- store updates after success

Stores and components can then be tested independently.

---

# Responsibilities

| Layer | Responsibility |
|---|---|
| Component | Express user intent |
| Page | Coordinate requests |
| Action | Execute GraphQL operations |
| HttpGraphQLClient | Communicate with the backend |
| Store | Own shared application state |

---

# Best Practices

- Execute GraphQL only from actions.
- Inject `HttpGraphQLClient`.
- Keep GraphQL out of components.
- Keep GraphQL out of pages.
- Update stores after successful operations.
- Convert responses into domain models.
- Handle failures inside actions.
- Test actions independently from the backend.

---

# What's Next?

GraphQL performs application operations.

The next guide explains how **Authentication & Authorization** protects those operations and controls access to application resources.
