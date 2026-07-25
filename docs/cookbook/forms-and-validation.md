# Forms & Validation

Forms collect user input and turn it into application operations.

A complete form workflow includes more than rendering input fields. It must also:

- preserve submitted values
- validate user input
- display useful error messages
- execute an action
- handle business errors
- update the user interface

A typical Drogular form follows this flow:

```text
User Input
    │
    ▼
Form Model
    │
    ▼
UI Validation
    │
    ▼
Action
    │
    ▼
Business Validation
    │
    ▼
Service or GraphQL Mutation
    │
    ▼
Success or Errors
    │
    ▼
UI Update
```

---

# Example Form

This guide uses a form for creating a todo item.

The form accepts a title and creates a new item when the submitted data is valid.

```cpp
struct CreateTodoForm
{
    std::string title;
};
```

A form model should contain only the data submitted by the user.

It should not perform network requests, access databases, or contain application business logic.

---

# Two Levels of Validation

Drogular applications should validate form data at two different levels:

1. UI validation
2. Business validation

These checks may overlap, but they serve different purposes.

---

# UI Validation

UI validation provides immediate and useful feedback to the user.

It typically checks:

- required fields
- minimum and maximum lengths
- basic formatting
- obvious input mistakes

For example, the todo title must not be empty:

```cpp
ValidationResult validateCreateTodoForm(const CreateTodoForm& form)
{
    ValidationResult result;

    if (form.title.empty())
    {
        result.addError("title", "Title is required.");
    }

    return result;
}
```

The page or component can use this result to render field-specific errors without executing the action.

```cpp
const auto validation = validateCreateTodoForm(form);

if (!validation.isValid())
{
    return renderForm(form, validation);
}
```

UI validation improves the experience, but it is not a security boundary.

Client input must never be trusted only because it passed form validation.

---

# Business Validation

Business validation protects the operation itself.

It belongs in the action or in a service called by the action.

Business validation may check:

- whether the current user may perform the operation
- whether a referenced resource exists
- whether the operation is allowed in the current state
- whether the submitted value conflicts with existing data
- domain-specific constraints

For example:

```cpp
class CreateTodoAction
{
public:
    explicit CreateTodoAction(std::shared_ptr<TodoService> todoService)
        : todoService_(std::move(todoService))
    {
    }

    ActionResult execute(
        const CreateTodoForm& form,
        const CurrentUser& currentUser)
    {
        if (!currentUser.canCreateTodos())
        {
            return ActionResult::failure(
                "You are not allowed to create todo items.");
        }

        if (todoService_->titleExists(form.title))
        {
            return ActionResult::fieldFailure(
                "title",
                "A todo item with this title already exists.");
        }

        todoService_->create(form.title);

        return ActionResult::success();
    }

private:
    std::shared_ptr<TodoService> todoService_;
};
```

The action validates the operation even when the page has already validated the form.

This is necessary because actions may be called from places other than the original form.

---

# ValidationResult

`ValidationResult` represents the validation state of submitted data.

It can contain:

- a valid or invalid state
- field-specific errors
- general form errors

A field error belongs to one input:

```cpp
result.addError("title", "Title is required.");
```

A general error applies to the complete operation:

```cpp
result.addError(
    "The todo item could not be created.");
```

Field errors should be displayed close to the corresponding input.

General errors should be displayed in a visible form-level message.

---

# FormValidator

Reusable validation rules should be placed in a validator rather than repeated in pages or components.

```cpp
class CreateTodoFormValidator
{
public:
    ValidationResult validate(const CreateTodoForm& form) const
    {
        ValidationResult result;

        if (form.title.empty())
        {
            result.addError("title", "Title is required.");
        }

        if (form.title.size() > 200)
        {
            result.addError(
                "title",
                "Title must not exceed 200 characters.");
        }

        return result;
    }
};
```

The validator should remain deterministic.

Given the same form data, it should return the same validation result.

Checks requiring databases, permissions, remote services, or application state belong in an action or service instead.

---

# Handling a Submission

A page coordinates the complete form submission.

It:

1. reads request values
2. creates the form model
3. runs UI validation
4. executes the action
5. handles the action result
6. returns the response

```cpp
HttpResponsePtr CreateTodoPage::submit(
    const HttpRequestPtr& request)
{
    CreateTodoForm form{
        .title = request->getParameter("title")
    };

    const auto validation = validator_->validate(form);

    if (!validation.isValid())
    {
        return renderForm(form, validation);
    }

    const auto result = action_->execute(
        form,
        currentUser_->get());

    if (!result.isSuccessful())
    {
        return renderForm(
            form,
            result.validation());
    }

    return redirect("/todos");
}
```

The page coordinates the request but does not implement the business operation itself.

---

# Preserve Submitted Values

When validation fails, the form should preserve the values entered by the user.

A user should not need to re-enter valid information because one field contains an error.

```cpp
return renderForm(form, validation);
```

The renderer receives both:

- the submitted form model
- the validation result

This allows it to restore values and display errors in the same response.

Sensitive values such as passwords should not normally be rendered back into the form.

---

# Rendering Field Errors

Each field should display its own validation errors.

Conceptually:

```html
<label for="title">Title</label>

<input
    id="title"
    name="title"
    value="{{ form.title }}"
>

{{#if errors.title}}
    <div class="field-error">
        {{ errors.title }}
    </div>
{{/if}}
```

The exact template syntax depends on the rendering layer used by the application.

The important requirement is that the error is associated with the field that caused it.

---

# General Form Errors

Some failures do not belong to a specific field.

Examples include:

- the service is temporarily unavailable
- the operation was rejected
- the submitted resource changed before the form was processed
- an unexpected remote error occurred

These errors should appear above or below the form as a general message:

```html
{{#if errors.form}}
    <div class="form-error">
        {{ errors.form }}
    </div>
{{/if}}
```

Do not attach an operation-wide failure to an unrelated field.

---

# Actions

A form should execute an action rather than directly calling business services from the component.

Actions represent user intentions such as:

- `CreateTodoAction`
- `UpdateProfileAction`
- `DeleteProjectAction`
- `SubmitOrderAction`

An action may:

- validate business rules
- call one or more services
- perform a GraphQL mutation
- update application state
- return structured success or failure information

This keeps the form workflow explicit and testable.

---

# GraphQL Mutations

When a form submits data through GraphQL, the action owns the mutation.

```cpp
class CreateTodoAction
{
public:
    explicit CreateTodoAction(
        std::shared_ptr<HttpGraphQLClient> graphqlClient)
        : graphqlClient_(std::move(graphqlClient))
    {
    }

    ActionResult execute(const CreateTodoForm& form)
    {
        CreateTodoVariables variables{
            .title = form.title
        };

        const auto response = graphqlClient_->execute(
            CreateTodoMutation,
            variables);

        if (!response)
        {
            return ActionResult::failure(
                "The todo item could not be created.");
        }

        return ActionResult::success();
    }

private:
    std::shared_ptr<HttpGraphQLClient> graphqlClient_;
};
```

The page and component should not contain GraphQL request code.

They only handle input, presentation, and the action result.

---

# Updating Application State

A successful action may update a store.

```text
Form
  │
  ▼
Action
  │
  ├── GraphQL Mutation
  │
  └── Store Update
          │
          ▼
      UI Re-render
```

For example, after creating a todo item, the action may add the returned item to `TodoStore`.

The component reacts to the changed state rather than manually duplicating the server response in local UI logic.

---

# Redirect After Success

For server-rendered forms, redirecting after a successful submission is usually preferable.

```cpp
return redirect("/todos");
```

This prevents the browser from submitting the same form again when the page is refreshed.

The pattern is commonly described as:

```text
POST
  │
  ▼
Action
  │
  ▼
Redirect
  │
  ▼
GET
```

Validation failures should render the form directly because the submitted values and validation errors must remain available.

---

# Dependency Injection

Validators and actions should be provided through dependency injection.

```cpp
class CreateTodoPage
{
public:
    CreateTodoPage(
        std::shared_ptr<CreateTodoFormValidator> validator,
        std::shared_ptr<CreateTodoAction> action,
        std::shared_ptr<CurrentUserProvider> currentUser)
        : validator_(std::move(validator)),
          action_(std::move(action)),
          currentUser_(std::move(currentUser))
    {
    }

private:
    std::shared_ptr<CreateTodoFormValidator> validator_;
    std::shared_ptr<CreateTodoAction> action_;
    std::shared_ptr<CurrentUserProvider> currentUser_;
};
```

This makes every dependency visible and allows validators and actions to be tested independently.

---

# Testing Validation

Validation rules should be tested without starting the complete application.

```cpp
TEST(CreateTodoFormValidator, RejectsEmptyTitle)
{
    CreateTodoFormValidator validator;

    const CreateTodoForm form{
        .title = ""
    };

    const auto result = validator.validate(form);

    EXPECT_FALSE(result.isValid());
    EXPECT_TRUE(result.hasError("title"));
}
```

Also test valid input:

```cpp
TEST(CreateTodoFormValidator, AcceptsValidTitle)
{
    CreateTodoFormValidator validator;

    const CreateTodoForm form{
        .title = "Write documentation"
    };

    const auto result = validator.validate(form);

    EXPECT_TRUE(result.isValid());
}
```

---

# Testing Actions

Action tests should focus on business behavior.

Typical cases include:

- the operation succeeds
- the user lacks permission
- the resource already exists
- the service returns an error
- the store is updated after success

```cpp
TEST(CreateTodoAction, RejectsDuplicateTitle)
{
    auto todoService = std::make_shared<FakeTodoService>();
    todoService->addExistingTitle("Write documentation");

    CreateTodoAction action(todoService);

    const CreateTodoForm form{
        .title = "Write documentation"
    };

    const auto result = action.execute(
        form,
        authorizedUser());

    EXPECT_FALSE(result.isSuccessful());
    EXPECT_TRUE(result.hasFieldError("title"));
}
```

UI validation tests do not replace action tests.

The two layers protect different responsibilities.

---

# Responsibilities

A form workflow should keep each responsibility in the appropriate layer.

| Layer | Responsibility |
|---|---|
| Form model | Submitted values |
| Component | Inputs and user feedback |
| Validator | Immediate input validation |
| Page | Request coordination |
| Action | Business operation |
| Service | Business logic and external access |
| Store | Shared application state |

---

# Best Practices

- Use a dedicated form model.
- Preserve submitted values after validation errors.
- Display field errors next to their fields.
- Use general errors for operation-wide failures.
- Perform fast input validation before executing the action.
- Repeat critical validation inside the business operation.
- Keep database and network checks out of form validators.
- Execute business operations through actions.
- Keep GraphQL requests out of components.
- Inject validators, actions, and services.
- Test validators and actions independently.
- Redirect after successful server-rendered submissions.

---

# What's Next?

Forms often modify shared application data.

The State Management guide explains how stores hold that data, notify subscribers, and keep the user interface synchronized after actions complete.
