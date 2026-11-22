# Principal & Identity

The `drogular::security` types provide a framework-independent identity model.
They do not authenticate credentials or grant permissions.

```cpp
#include <drogular/security/principal_builder.hpp>

auto result = drogular::security::PrincipalBuilder{}
    .issuer("https://identity.example")
    .subject("user-123")
    .type(drogular::security::PrincipalType::User)
    .role("editor")
    .build();

if (!result) {
    // Handle result.error().code / result.error().message.
}
```

`PrincipalId` pairs issuer and subject; the subject is not globally unique.
Issuer normalization and identity verification are responsibilities of a future
trusted authentication adapter. `PrincipalBuilder` only validates structural
invariants, and does not make caller-provided values trustworthy.

`Principal` is immutable through its public interface and can be copied or moved.
Roles are unscoped identity metadata, **not** authorization grants; repeated
roles are deduplicated preserving first occurrence. Blank identity values and
roles produce typed errors through the C++20 `drogular::Expected` API.

## Typed claims and attributes

`Principal::claims()` and `Principal::attributes()` expose separate, read-only
snapshots. Both use `ClaimValue = std::variant<bool, std::int64_t, double,
std::string, std::vector<std::string>>`. `get<T>(name)` performs an **exact**
type lookup and returns `std::optional<T>` (no conversions); `find(name)`
returns a pointer to the variant or `nullptr` if the name is absent.

```cpp
using namespace drogular::security;
auto result = PrincipalBuilder{}
    .issuer("identity-provider").subject("user-42")
    .claim("department", std::string{"finance"})
    .attribute("locale", std::string{"de"})
    .build();
if (result) {
    auto department = result->claims().get<std::string>("department");
    auto locale = result->attributes().get<std::string>("locale");
}
```

Empty/whitespace-only names and duplicate names within either namespace are
rejected with `PrincipalError`. The reserved identity claims `iss` and `sub`
are permitted only as strings matching the PrincipalId values exactly.
The two namespaces do not overlap: a name may appear in both.

**Trust boundary:** `Claims` contains caller-supplied values, not automatically
verified claims. Only a future authentication adapter may establish their
trustworthiness; `Attributes` must never be implicitly used as trusted identity
claims. Neither class offers public mutations after construction.

## SecurityContext

`drogular::security::SecurityContext` is an HTTP-independent identity snapshot.
A default-constructed context (or `SecurityContext::anonymous()`) has no
Principal and reports `authenticated() == false`. An identity can be attached
using `SecurityContext::forPrincipal(Principal, optional<string> method)`.
This factory **does not verify credentials**: it is the responsibility of a
future trusted authentication adapter to establish identity before creating a
context. Applications must not treat self-asserted contexts as authenticated
proof. The optional method is descriptive metadata, not a credential.

```cpp
#include <drogular/security/security_context.hpp>

auto context = drogular::security::SecurityContext::forPrincipal(
    std::move(result.value()), std::string{"session"});
if (context.authenticated()) {
    auto identity = context.principal(); // shared_ptr<const Principal>
}
```

Copies share a read-only Principal snapshot that outlives an HTTP request when
retained by an asynchronous operation. No token, password, session cookie,
or authorization decision is stored. `authenticationMethod()` returns an
optional non-owning string view valid only while the corresponding context
is alive and unchanged. For detached storage, copy the string.

## Request integration

`ActionContext::securityContext()` and `RenderContext::securityContext()`
return a lightweight copy of the shared request identity snapshot.
`principal()` is a read-only convenience accessor returning
`std::shared_ptr<const security::Principal>` (null for anonymous requests).
Both accessors are read-only and do **not** authenticate the request.

The internal `detail::RequestContextBridge::bindSecurityContext(action, context)`
can be used by framework integration code after verification by a trusted
adapter (planned for next Stage). It must **not** be used to treat unverified
request claims or headers as authenticated credentials. The snapshot is
shared from an action into its render context and child render contexts, but
not across independently constructed ActionContexts. Detached copies keep
Principal alive after the original HTTP request ends; they do not carry
permissions or guarantee that roles remain current.

This stage does not add a `drogon::HttpRequest` extension or a global current
user. Third-party Drogon controllers remain outside the action/render
integration until a separately designed request adapter is introduced.
