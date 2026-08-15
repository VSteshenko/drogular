# Protecting Actions

## Problem

**Need to prevent guests or unauthorized users from executing a command?**

Use `ActionAuthSupport` at the beginning of the action handler and return immediately when an access requirement fails.

This is especially important for actions that change application state: creating records, updating settings, deleting data, or executing privileged operations.

The Portal Demo uses this pattern for administrative commands such as creating, updating, and deleting roles or project types.

---

## Recommended Solution

Treat authentication and authorization as part of the action boundary.

A protected action should follow this sequence:

```text
Action::handle()
    │
    ▼
Require authentication
    │
    ├── denied ──► return redirect result
    │
    ▼ allowed
Require authorization, if needed
    │
    ├── denied ──► return redirect result
    │
    ▼ allowed
Validate command input
    │
    ├── invalid ─► return validation result
    │
    ▼ valid
Execute protected mutation
    │
    ▼
Return success result
```

The important rule is that no protected work happens before every access requirement succeeds.

---

## Require Authentication at the Action Boundary

`ActionAuthSupport::requireAuthentication()` checks the session associated with the current `ActionContext`.

```cpp
if (const auto denied =
        drogular::ActionAuthSupport::requireAuthentication(
            context
        )) {
    return *denied;
}

// Safe to continue with work that only requires
// the framework authentication baseline.
```

The method returns:

- `std::nullopt` when authentication succeeds;
- an `ActionResult` redirect when authentication fails.

The default redirect target is `/login`.

This makes the action contract explicit:

```cpp
if (const auto denied = requirement(context)) {
    return *denied;
}

// Protected command begins here.
```

Unlike `PageAuthSupport`, the action helper does not set presentation state. It produces the result the action should return.

Do not ignore that result and continue executing the handler.

---

## Framework Authentication Is the Baseline

`ActionAuthSupport::requireAuthentication()` delegates to the same framework rule used by page authentication.

A request is considered authenticated when its resolved session contains `username`.

That gives Drogular a common baseline, but applications may require a stronger identity contract.

For example, if a command requires both a username and a role, authentication alone is not enough. Add the relevant authorization requirement before executing the mutation.

---

## Add Role-Based Authorization

`ActionAuthSupport::requireSessionValue()` requires a specific value in the existing session.

For an admin-only command:

```cpp
if (const auto denied =
        drogular::ActionAuthSupport::requireAuthentication(
            context
        )) {
    return *denied;
}

if (const auto denied =
        drogular::ActionAuthSupport::requireSessionValue(
            context,
            "role",
            "admin",
            "/roles?error=access_denied"
        )) {
    return *denied;
}

// Admin-only command.
```

The two checks answer different questions:

```text
requireAuthentication()
        │
        └── Is there an authenticated user?

requireSessionValue("role", "admin", ...)
        │
        └── Does the current session satisfy this access rule?
```

Keep these checks separate. Authentication establishes identity; authorization evaluates what that identity may do.

---

## Protect a Real Command

Portal Demo protects role creation before validation or provider access.

A simplified version looks like this:

```cpp
class PortalCreateRoleAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        if (const auto denied =
                drogular::ActionAuthSupport::
                    requireAuthentication(context)) {
            return *denied;
        }

        if (const auto denied =
                drogular::ActionAuthSupport::
                    requireSessionValue(
                        context,
                        "role",
                        "admin",
                        "/roles?error=access_denied"
                    )) {
            return *denied;
        }

        const auto validation =
            drogular::FormValidator(context)
                .required("code")
                .minLength("code", 2)
                .required("title")
                .minLength("title", 2)
                .validate();

        if (!validation.valid()) {
            return drogular::ActionResult::redirect(
                "/roles?error=validation"
            );
        }

        auto roles =
            context.requireService<PortalRoleProvider>();

        PortalRoleCreate input;
        input.code = context.requireForm<std::string>("code");
        input.title = context.requireForm<std::string>("title");

        roles->create(input);

        return drogular::ActionResult::redirect(
            "/roles?success=role_created"
        );
    }
};
```

Notice the ordering:

```text
access checks
    ↓
input validation
    ↓
service lookup
    ↓
mutation
```

This ordering keeps unauthorized requests away from protected application work.

---

## Why Authorization Comes Before Validation

A protected command should normally establish access before processing its business input.

Prefer:

```cpp
if (const auto denied = requireAccess(context)) {
    return *denied;
}

const auto validation = validate(context);
```

instead of:

```cpp
const auto validation = validate(context);

if (const auto denied = requireAccess(context)) {
    return *denied;
}
```

There are two reasons.

First, authorization is part of the command boundary. A caller who is not allowed to execute the command should not reach its business workflow.

Second, validating or otherwise inspecting protected command semantics before authorization can expose unnecessary behavior differences to unauthorized callers.

Request parsing performed by the HTTP stack still happens normally, but application-level validation and mutation logic should begin after access is established.

---

## Return the Denial Result Immediately

This is correct:

```cpp
if (const auto denied =
        drogular::ActionAuthSupport::requireAuthentication(
            context
        )) {
    return *denied;
}
```

This is not:

```cpp
drogular::ActionAuthSupport::requireAuthentication(context);

provider->remove(id); // May still run for a guest.
```

`ActionAuthSupport` does not terminate the handler itself.

It returns an `ActionResult` that the handler must return.

The same rule applies to `requireSessionValue()`.

---

## Use Different Redirects for Authentication and Authorization

Authentication failure and authorization failure represent different states.

A useful pattern is:

```cpp
if (const auto denied =
        drogular::ActionAuthSupport::requireAuthentication(
            context,
            "/login"
        )) {
    return *denied;
}

if (const auto denied =
        drogular::ActionAuthSupport::requireSessionValue(
            context,
            "role",
            "admin",
            "/roles?error=access_denied"
        )) {
    return *denied;
}
```

A guest is sent to login.

An authenticated user without the required role is sent back to an application page that can display an access-denied message.

This mirrors the distinction between `loginRequired` and `accessDenied` used by protected pages.

---

## Missing Session Values Fail Closed

`requireSessionValue()` denies access when:

- there is no existing session;
- the requested key is missing;
- the value differs from the expected value.

For example:

```cpp
if (const auto denied =
        drogular::ActionAuthSupport::requireSessionValue(
            context,
            "role",
            "admin",
            "/dashboard"
        )) {
    return *denied;
}
```

A session without `role` does not fall through as if it were authorized.

That fail-closed behavior is important for sessions that are stale, incomplete, or created by older versions of an application.

---

## Do Not Create a Session While Checking Access

Protected commands should inspect the existing session, not create a new anonymous one as a side effect of authorization.

`ActionAuthSupport` follows this rule:

- authentication resolves the existing request session through `AuthSupport`;
- `requireSessionValue()` uses `ActionContext::existingSession()`;
- missing session state causes denial.

This keeps access checks read-only with respect to session creation.

Session creation belongs to flows such as successful login, not to protected command execution.

---

## Keep Authorization Out of the Mutation Body

Avoid burying access checks next to business operations:

```cpp
auto roles = context.requireService<PortalRoleProvider>();

if (isAdmin(context)) {
    roles->create(input);
}
```

This makes it too easy to add protected work before the check or forget the check on another code path.

Prefer a boundary-first structure:

```cpp
if (const auto denied = requireAdmin(context)) {
    return *denied;
}

auto roles = context.requireService<PortalRoleProvider>();
roles->create(input);
```

Once execution passes the access boundary, the rest of the handler can focus on the command itself.

---

## Extract Repeated Application Rules

If many actions use the same policy, wrap the framework helpers in an application-level helper instead of repeating session keys everywhere.

For example:

```cpp
class AdminActionSupport {
public:
    static std::optional<drogular::ActionResult> requireAdmin(
        drogular::ActionContext& context
    ) {
        if (const auto denied =
                drogular::ActionAuthSupport::
                    requireAuthentication(context)) {
            return denied;
        }

        return drogular::ActionAuthSupport::requireSessionValue(
            context,
            "role",
            "admin",
            "/dashboard?error=access_denied"
        );
    }
};
```

Then an action becomes:

```cpp
if (const auto denied =
        AdminActionSupport::requireAdmin(context)) {
    return *denied;
}

// Protected command.
```

This is useful when the application's identity model becomes richer than a single role value.

The framework helper remains small and generic; the application helper expresses the application's actual policy.

---

## Protect Every Mutating Entry Point

Hiding an admin button in a page does not protect the corresponding action.

A caller can send a POST request directly.

Therefore both layers should enforce their own contracts:

```text
Protected page
    └── controls what the user can see

Protected action
    └── controls what the user can execute
```

The action is the security boundary for the mutation.

Never rely on the page that rendered the form to guarantee that the caller is authorized when the form is submitted.

---

## Testing Protected Actions

Test the action contract directly.

At minimum, cover:

```text
guest
    └── protected action
        └── redirected to login

authenticated regular user
    └── admin action
        └── redirected with access denied

authenticated admin
    └── admin action
        └── mutation executes

session missing required value
    └── protected action
        └── denied
```

Drogular's core tests also verify the helper behavior itself.

For a guest:

```cpp
const auto result =
    drogular::ActionAuthSupport::requireAuthentication(
        context
    );

ASSERT_TRUE(result.has_value());
EXPECT_EQ(
    result->type(),
    drogular::ActionResultType::Redirect
);
EXPECT_EQ(result->location(), "/login");
```

For a session-value requirement:

```cpp
const auto result =
    drogular::ActionAuthSupport::requireSessionValue(
        context,
        "role",
        "admin",
        "/users?error=access_denied"
    );

ASSERT_TRUE(result.has_value());
EXPECT_EQ(
    result->location(),
    "/users?error=access_denied"
);
```

Application tests should go further and verify that denied requests do **not** execute the protected mutation.

That assertion matters more than checking the redirect alone.

---

## Common Mistakes

### Ignoring the returned result

```cpp
ActionAuthSupport::requireAuthentication(context);
```

The handler continues executing.

Instead:

```cpp
if (const auto denied =
        ActionAuthSupport::requireAuthentication(context)) {
    return *denied;
}
```

### Checking authorization only in the page

A hidden form or button does not protect its POST endpoint.

Protect the action independently.

### Mutating before authorization

```cpp
auto service = context.requireService<Service>();
service->prepareMutation();

if (const auto denied = requireAdmin(context)) {
    return *denied;
}
```

Move all protected work after the access boundary.

### Treating authentication as authorization

A valid `username` does not imply admin privileges.

Require the specific application rule needed by the command.

### Assuming a missing role is harmless

Missing authorization state must deny access, not silently fall back to a privileged path.

---

## Mental Model

For protected actions, think in terms of a guarded command boundary:

```text
Request
    │
    ▼
Authentication
    │
    ├── fail ──► return login redirect
    │
    ▼
Authorization
    │
    ├── fail ──► return access-denied redirect
    │
    ▼
Validation
    │
    ├── fail ──► return validation result
    │
    ▼
Mutation
```

Once the mutation begins, authentication and authorization should already be settled.

---

## See Also

- [Authentication Overview](overview.md)
- [Login & Logout](login-logout.md)
- [Protecting Pages](protecting-pages.md)
- [API Reference — `ActionAuthSupport`](../../reference/authentication-and-sessions/action-auth-support.md)
- [API Reference — `AuthSupport`](../../reference/authentication-and-sessions/auth-support.md)
- [API Reference — `ActionContext`](../../reference/actions/action-context.md)
- [API Reference — `ActionResult`](../../reference/actions/action-result.md)
- [Forms & Validation](../forms-and-validation.md)

---

## Next

Continue with **Role-Based Authorization** to move from individual `role=admin` checks toward a clear application authorization model shared by pages and actions.
