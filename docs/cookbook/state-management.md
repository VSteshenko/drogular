# State Management

State Management keeps application data synchronized across pages and components.

Instead of each component storing its own copy of shared data, Drogular uses stores as the single source of truth.

A store owns application state and notifies interested components whenever that state changes.

---

# Why State Management?

Without shared state, components quickly become difficult to keep synchronized.

For example:

```text
TodoList
    │
    ├── stores 10 todos
    │
Sidebar
    │
    ├── stores 10 todos
    │
Dashboard
    │
    └── stores 10 todos
```

If one component updates the data, every other component must also be updated.

This approach does not scale.

Instead, all components observe the same store.

```text
                TodoStore
                    │
        ┌───────────┼───────────┐
        ▼           ▼           ▼
    TodoList     Sidebar    Dashboard
```

The store owns the data.

Components only render it.

---

# Single Source of Truth

Every piece of shared application state should have one owner.

For example:

- `TodoStore`
- `UserStore`
- `NotificationStore`
- `ShoppingCartStore`

Other classes should read state from the store instead of maintaining their own copies.

---

# What Belongs in a Store?

A store should contain state that is shared across multiple parts of the application.

Examples include:

- authenticated user
- todo list
- shopping cart
- selected language
- notifications
- application settings

Local UI state should remain inside the component.

Examples include:

- dialog visibility
- expanded panels
- selected tab
- input focus
- temporary form values

---

# Store Responsibilities

A store is responsible for:

- owning shared state
- exposing read access
- notifying subscribers
- applying state changes

A store should not:

- perform GraphQL requests
- access databases
- contain business logic
- validate business rules

Those responsibilities belong to actions and services.

---

# Example Store

```cpp
class TodoStore
{
public:
    const std::vector<Todo>& todos() const
    {
        return todos_;
    }

    void add(Todo todo);

    void remove(TodoId id);

    void toggle(TodoId id);

private:
    std::vector<Todo> todos_;
};
```

The store owns the todo collection.

Components cannot modify the state directly.

---

# Reading State

Components read data from the store.

```cpp
const auto& todos = todoStore_->todos();
```

The component renders the state but does not own it.

---

# Updating State

State changes should happen through explicit store operations.

```cpp
todoStore_->add(todo);

todoStore_->toggle(id);

todoStore_->remove(id);
```

Avoid exposing mutable containers directly.

This keeps state changes predictable.

---

# Store Notifications

Whenever state changes, the store notifies its subscribers.

```text
Store Updated
      │
      ▼
Subscribers Notified
      │
      ▼
Components Re-render
```

Components react to state changes automatically instead of manually refreshing themselves.

---

# Actions Update Stores

Actions coordinate business operations.

When an operation succeeds, the action updates the appropriate store.

```text
User Click
      │
      ▼
Action
      │
      ▼
Service / GraphQL
      │
      ▼
Store Update
      │
      ▼
UI Refresh
```

The component never updates shared state directly after a business operation.

---

# Example Workflow

Creating a todo item typically follows this sequence:

```text
CreateTodoForm
        │
        ▼
CreateTodoAction
        │
        ▼
GraphQL Mutation
        │
        ▼
TodoStore.add(todo)
        │
        ▼
TodoList Re-renders
```

Every component observing `TodoStore` immediately receives the updated state.

---

# Keep Stores Simple

A store should represent state rather than behavior.

Good examples:

- add item
- remove item
- replace collection
- clear state

Poor examples:

- login user
- execute GraphQL mutation
- send email
- create database connection

Business operations belong elsewhere.

---

# Replacing State

Sometimes an entire collection is replaced.

For example, after loading data from GraphQL.

```cpp
todoStore_->setTodos(response.todos);
```

Replacing the complete state is often simpler than synchronizing many individual changes.

---

# Immutable Thinking

Although Drogular is written in C++, stores should behave as if state changes are intentional events.

Rather than modifying state from many places:

```text
Component A
      │
Component B
      │
Component C
      │
      ▼
Shared Vector
```

Prefer:

```text
Component
    │
    ▼
Store Method
    │
    ▼
State Updated
```

This makes changes easier to understand and debug.

---

# Dependency Injection

Stores should be provided through dependency injection.

```cpp
class TodoPage
{
public:
    TodoPage(
        std::shared_ptr<TodoStore> store)
        : store_(std::move(store))
    {
    }

private:
    std::shared_ptr<TodoStore> store_;
};
```

Every component using the same injected store observes the same shared state.

---

# Testing Stores

Stores should be easy to test independently.

Typical tests include:

- adding items
- removing items
- replacing state
- notification delivery
- subscriber ordering

Example:

```cpp
TEST(TodoStore, AddsTodo)
{
    TodoStore store;

    store.add(Todo{"Write documentation"});

    EXPECT_EQ(store.todos().size(), 1);
}
```

A store test should not require GraphQL or a running application.

---

# Store Lifetime

Stores are typically registered as singletons.

This allows multiple pages and components to share the same application state.

A different lifetime may be appropriate for specialized scenarios, but singleton stores are the recommended default.

---

# Best Practices

- One store owns one shared state.
- Keep stores focused on data.
- Read state from stores.
- Modify state through store methods.
- Let actions update stores.
- Keep business logic out of stores.
- Keep GraphQL requests out of stores.
- Use dependency injection.
- Test stores independently.

---

# Responsibilities

| Layer | Responsibility |
|---|---|
| Component | Render shared state |
| Store | Own shared state |
| Action | Execute business operations |
| Service | Business logic |
| GraphQL | External communication |

---

# What's Next?

Now that you understand shared state, the next guide explains how Actions encapsulate user intentions and coordinate business operations across services, GraphQL, and stores.
