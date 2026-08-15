# Login & Logout

## Problem

**Need to authenticate a user, establish a server-side session, and terminate it safely on logout?**

This guide shows the recommended login and logout flow using Drogular actions and `SessionStore`.

The complete example is available in `examples/auth_sample`.

---

## Recommended Solution

Treat login as a boundary between unauthenticated input and authenticated session state:

1. validate the submitted credentials;
2. authenticate them in application code;
3. remove any previously supplied session;
4. create a fresh session;
5. store the application identity in that session;
6. return the new session identifier in a cookie.

Logout performs the inverse operation:

1. resolve the session identifier from the request cookie;
2. remove the server-side session;
3. expire the client cookie;
4. redirect to a public page.

```text
Login form
    │
    ▼
Validate input
    │
    ▼
Authenticate credentials
    │
    ├── invalid ──► return to login
    │
    ▼ valid
Remove old session
    │
    ▼
Create fresh session
    │
    ▼
Store identity
    │
    ▼
Set session_id cookie
    │
    ▼
Protected application
```

The application owns credential verification and identity design. Drogular provides action handling, dependency injection, session storage, and response cookies.

---

## Login Action

`auth_sample` implements login as an `ActionHandler` registered on `POST /login`.

```cpp
app.action<LoginAction>("/login");
```

The page that displays the form and the action that processes credentials remain separate:

```cpp
app.page<LoginPage>("/login");
app.action<LoginAction>("/login");
```

This keeps rendering and command handling independent even though they share the same URL.

---

## Validate Input Before Authentication

Validate required form fields before calling the authentication service.

```cpp
const auto validation =
    drogular::FormValidator(context)
        .required("username")
        .required("password")
        .validate();

if (!validation.valid()) {
    return drogular::ActionResult::redirect("/login");
}
```

Only after validation succeeds should the action read required typed values:

```cpp
const auto user =
    authService->authenticate(
        context.requireForm<std::string>("username"),
        context.requireForm<std::string>("password")
    );
```

The authentication service is application code. Drogular does not prescribe a user repository, password hashing library, LDAP integration, OAuth provider, or other credential backend.

---

## Reject Invalid Credentials Without Creating a Session

An authentication failure should stop before any authenticated session is created.

```cpp
if (!user.has_value()) {
    return drogular::ActionResult::redirect("/login");
}
```

In `auth_sample`, invalid credentials produce no session cookie and do not create a new authenticated session.

This ordering matters. Session rotation belongs to the successful authentication path, not to every submitted login attempt.

---

## Rotate the Session After Login

After credentials are successfully authenticated, do not reuse an existing session identifier as the authenticated session.

`auth_sample` first removes any session referenced by the incoming cookie:

```cpp
auto sessionStore =
    context.requireService<drogular::SessionStore>();

if (const auto existingSessionId =
        context.cookie("session_id")) {
    sessionStore->remove(*existingSessionId);
}
```

It then creates a fresh session:

```cpp
auto session = sessionStore->create();
```

This produces a new session identifier for the authenticated identity.

```text
Existing session_id
      │
      ▼
credentials valid?
      │
      ├── no  ──► keep login unauthenticated
      │
      ▼ yes
remove old session
      │
      ▼
create new session_id
      │
      ▼
authenticated identity
```

Rotating the identifier prevents an unauthenticated session identifier from being promoted into an authenticated session, which is the key defense against session fixation in this flow.

---

## Store the Application Identity

The application decides which identity values belong in the session.

`auth_sample` stores the user identifier, username, and role:

```cpp
session->set(
    "user_id",
    std::to_string(user->id)
);

session->set(
    "username",
    user->username
);

session->set(
    "role",
    user->role
);
```

For Drogular's built-in authentication helpers, `username` is the shared baseline value used by `AuthSupport::isAuthenticated()`.

The sample defines a stricter application contract: its `AuthSession` requires both `username` and `role` before constructing an `AuthUser`.

Store only the identity and authorization data needed by the application. Avoid putting credentials, plaintext passwords, or other secrets in session values.

---

## Issue the Session Cookie

The session identifier is stored internally under `_id`:

```cpp
const auto sessionId =
    session->get("_id").value();
```

Return it to the browser as the `session_id` cookie:

```cpp
return drogular::ActionResult::redirect("/dashboard")
    .cookie(
        "session_id",
        sessionId,
        drogular::CookieOptions{
            .httpOnly = true,
            .secure = false,
            .sameSite = drogular::CookieSameSite::Lax
        }
    );
```

For the local `auth_sample`, `Secure` remains disabled because the example runs over plain HTTP.

For production HTTPS deployments, enable `Secure`:

```cpp
drogular::CookieOptions{
    .httpOnly = true,
    .secure = true,
    .sameSite = drogular::CookieSameSite::Lax
}
```

`HttpOnly` prevents client-side JavaScript from reading the session cookie. `SameSite=Lax` provides a useful baseline against cross-site cookie sending while remaining compatible with normal top-level navigation.

Cookie policy is application deployment policy; choose attributes appropriate for the actual HTTPS, domain, path, and cross-site requirements of the application.

---

## Complete Login Action

The sample login action combines validation, application authentication, session rotation, identity storage, and cookie issuance:

```cpp
class LoginAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        const auto validation =
            drogular::FormValidator(context)
                .required("username")
                .required("password")
                .validate();

        if (!validation.valid()) {
            return drogular::ActionResult::redirect("/login");
        }

        auto authService =
            context.requireService<AuthService>();

        const auto user =
            authService->authenticate(
                context.requireForm<std::string>("username"),
                context.requireForm<std::string>("password")
            );

        if (!user.has_value()) {
            return drogular::ActionResult::redirect("/login");
        }

        auto sessionStore =
            context.requireService<drogular::SessionStore>();

        if (const auto existingSessionId =
                context.cookie("session_id")) {
            sessionStore->remove(*existingSessionId);
        }

        auto session = sessionStore->create();

        session->set("user_id", std::to_string(user->id));
        session->set("username", user->username);
        session->set("role", user->role);

        const auto sessionId =
            session->get("_id").value();

        return drogular::ActionResult::redirect("/dashboard")
            .cookie(
                "session_id",
                sessionId,
                drogular::CookieOptions{
                    .httpOnly = true,
                    .secure = false,
                    .sameSite = drogular::CookieSameSite::Lax
                }
            );
    }
};
```

In a real application, credential failures will usually need user-visible feedback rather than an unconditional redirect. That presentation policy is independent of the session establishment flow shown here.

---

## Logout Action

Logout must remove server-side authentication state, not merely clear the browser cookie.

Read the session identifier from the request:

```cpp
auto sessionId =
    context.cookie("session_id");
```

If present, remove the corresponding session:

```cpp
if (sessionId.has_value()) {
    auto sessionStore =
        context.requireService<drogular::SessionStore>();

    sessionStore->remove(*sessionId);
}
```

Then expire the browser cookie:

```cpp
return drogular::ActionResult::redirect("/login")
    .cookie(
        "session_id",
        "",
        drogular::CookieOptions{
            .httpOnly = true,
            .secure = false,
            .sameSite = drogular::CookieSameSite::Lax,
            .maxAge = 0
        }
    );
```

Setting an empty value and `Max-Age=0` tells the browser to remove the cookie immediately.

```text
Logout request
      │
      ▼
session_id cookie
      │
      ├── present ──► SessionStore::remove()
      │
      ▼
expire client cookie
      │
      ▼
redirect to /login
```

The logout flow remains safe when no session cookie is present: there is no server-side session to remove, but the response still expires the cookie and redirects to the login page.

---

## Complete Logout Action

```cpp
class LogoutAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        auto sessionId =
            context.cookie("session_id");

        if (sessionId.has_value()) {
            auto sessionStore =
                context.requireService<drogular::SessionStore>();

            sessionStore->remove(*sessionId);
        }

        return drogular::ActionResult::redirect("/login")
            .cookie(
                "session_id",
                "",
                drogular::CookieOptions{
                    .httpOnly = true,
                    .secure = false,
                    .sameSite = drogular::CookieSameSite::Lax,
                    .maxAge = 0
                }
            );
    }
};
```

---

## SessionStore Registration

The login and logout actions resolve `SessionStore` through dependency injection, so the application must register it:

```cpp
app.services().add<drogular::SessionStore>(
    drogular::ServiceLifetime::Singleton
);
```

`AuthService` is registered in the same way in the sample:

```cpp
app.services().add<AuthService>(
    drogular::ServiceLifetime::Singleton
);
```

The built-in `SessionStore` is in-memory. Session persistence, expiration, distributed storage, and multi-process sharing require an application-specific solution when those properties are needed.

---

## Testing the Flow

Authentication tests should verify security behavior, not only redirects.

`auth_sample` tests that a valid login:

- redirects to `/dashboard`;
- returns exactly one `session_id` cookie;
- uses `HttpOnly` and `SameSite=Lax`;
- creates a server-side session;
- stores the expected identity values.

It also verifies session rotation:

```cpp
EXPECT_NE(newSessionId, oldSessionId);
EXPECT_FALSE(store->contains(oldSessionId));
EXPECT_TRUE(store->contains(newSessionId));
```

Invalid credentials must not issue a session cookie:

```cpp
EXPECT_EQ(result.location(), "/login");
EXPECT_TRUE(result.cookies().empty());
```

Logout tests verify both halves of termination:

```cpp
EXPECT_FALSE(store->contains(sessionId));
```

and:

```cpp
EXPECT_EQ(result.cookies()[0].value, "");
ASSERT_TRUE(result.cookies()[0].maxAge.has_value());
EXPECT_EQ(*result.cookies()[0].maxAge, 0);
```

Testing both server-side removal and client-side expiration prevents a superficially correct logout from leaving authenticated state behind.

---

## Best Practices

- Validate login input before reading required form values.
- Authenticate credentials in application code, not in templates or page rendering.
- Do not create an authenticated session until credential verification succeeds.
- Rotate the session identifier after successful login.
- Remove the previous server-side session during rotation.
- Store only the identity and authorization values the application actually needs.
- Never store plaintext passwords or credentials in session values.
- Use `HttpOnly` for session cookies.
- Enable `Secure` for production applications served over HTTPS.
- Choose an explicit `SameSite` policy instead of relying on browser defaults.
- On logout, remove the server-side session and expire the client cookie.
- Test session rotation and logout state removal, not only HTTP redirects.

---

## Next Steps

Continue with **Protecting Pages** to use the authenticated session as an explicit page-level access contract.

---

## See Also

### Example

- `examples/auth_sample/login_action.hpp`
- `examples/auth_sample/logout_action.hpp`
- `examples/auth_sample/auth_service.hpp`

### Cookbook

- [Authentication Overview](overview.md)
- [Forms & Validation](../forms-and-validation.md)

### API Reference

- [`Session`](../../reference/authentication-and-sessions/session.md)
- [`SessionStore`](../../reference/authentication-and-sessions/session-store.md)
