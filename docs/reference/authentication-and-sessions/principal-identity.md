# Principal and SecurityContext

The C++20 `drogular::security` model represents an **identity snapshot**, not a
login mechanism, verified credential, or authorization decision. This API is
independent of HTTP and Drogon. See also the
[architecture and trust boundaries](../../architecture/principal-identity.md).

## Headers and public types

| Header | Public contract |
| --- | --- |
| `<drogular/security/principal_id.hpp>` | `PrincipalId` (`issuer`, `subject`) |
| `<drogular/security/principal.hpp>` | `Principal`, `PrincipalType`, `ClaimValue`, `Claims`, `Attributes` |
| `<drogular/security/principal_builder.hpp>` | `PrincipalBuilder`, `PrincipalError`, `PrincipalErrorCode` |
| `<drogular/security/security_context.hpp>` | `SecurityContext` |
| `<drogular/expected.hpp>` | `Expected<T, E>`, `Unexpected<E>` (C++20) |

`PrincipalType` is `User`, `Service` or `System`. `PrincipalId` is scoped by
**both** `issuer` and `subject`; do not treat the subject as globally unique.
An absent `Principal` represents an anonymous context.

## Construct a Principal

```cpp
#include <drogular/security/principal_builder.hpp>

using namespace drogular::security;
auto built = PrincipalBuilder{}
    .issuer("identity-provider")
    .subject("user-42")
    .type(PrincipalType::User)
    .role("editor")
    .claim("department", std::string{"finance"})
    .attribute("locale", std::string{"de"})
    .build(); // drogular::Expected<Principal, PrincipalError>

if (!built) {
    // Inspect built.error().code and built.error().message.
    // Do not access built.value() here.
    return;
}
```

`build()` is rvalue-qualified (`&&`) and consumes its builder. Validation
rejects blank `issuer`/`subject`/roles, blank claim or attribute names,
duplicate names within the same namespace, and reserved `iss`/`sub` values
that do not match the id. Repeated roles are deduplicated in insertion order.
`Principal` has no public mutators.

`ClaimValue` supports **exactly** `bool`, `std::int64_t`, `double`,
`std::string`, and `std::vector<std::string>`. `claims().get<T>(name)` and
`attributes().get<T>(name)` return `std::optional<T>`; incorrect type or
missing key returns no value, with no implicit conversions. `contains()` and
`find()` allow existence and variant inspection. Claims and Attributes are
separate; neither becomes trustworthy merely by being placed in a Principal.

## SecurityContext

```cpp
#include <drogular/security/security_context.hpp>

SecurityContext anonymous; // or SecurityContext::anonymous()
// A verified adapter may construct and attach this snapshot:
auto identity = SecurityContext::forPrincipal(
    std::move(built.value()), std::string{"session"});

if (identity.authenticated()) {
    auto principal = identity.principal(); // shared_ptr<const Principal>
}
```

`forPrincipal` does **not** perform authentication. The optional
`authenticationMethod()` is descriptive, never a token or password. Copies
share immutable Principal ownership, so a copy survives its originating
request. Its `optional<string_view>` must not outlive the owning context.

## Request access

`ActionContext::securityContext()` and `RenderContext::securityContext()`
return independent copies of the request identity snapshot. Their
`principal()` shortcuts return `shared_ptr<const Principal>` (or null).
Child render contexts share the request state. Application code must not
use raw HTTP headers to assert a verified Principal; binding is an internal
framework concern (`detail::RequestContextBridge`) until Stage D adds an
authentication adapter. There is no global current user or automatic
conversion of existing session helpers to Principal.

## Explicit background attribution

```cpp
const auto identity = action.securityContext();
auto id = jobs.submit("rebuild-index", [](drogular::JobContext& job) {
    auto initiator = job.initiator(); // null for anonymous Jobs
    // For attribution only; recheck current permissions for privileged work.
}, identity);
```

Both `submit` and `submitResumable` have explicit third-argument
`SecurityContext` overloads. Without that argument Jobs remain anonymous.
A submitted snapshot can remain alive until terminal retention/pruning;
avoid unnecessary personal data. `JobSnapshot`, SSE, and diagnostics do not
expose the identity. No permission, tenant scope, or credential is propagated.

**Not included:** credential validation, JWT/session adapters,
role/permission enforcement, tenant authorization and policy evaluation.
These belong to next Stage and later releases.
