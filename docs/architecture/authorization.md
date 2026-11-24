# Authorization decisions

`<drogular/security/authorization.hpp>` provides an HTTP-independent,
fail-closed authorization policy based on the Stage C security snapshot.
This is the first Stage D increment; it does not replace the existing
session-based `AuthSupport`, `PageAuthSupport`, or `ActionAuthSupport`.

```cpp
using namespace drogular::security;
const auto decision = Authorization::requireRole(
    action.securityContext(), "editor");
if (decision != AuthorizationDecision::Allowed) {
    // Map AuthenticationRequired to 401 and Forbidden to 403 at the HTTP edge.
}
```

The checks return `Allowed`, `AuthenticationRequired`, or `Forbidden`.
Anonymous identities yield `AuthenticationRequired`; authenticated identities
without the exact requested role or permission yield `Forbidden`. Empty names
are never authorized. Roles match exactly and **do not** grant permissions.

`requirePermission()` checks a `permissions` claim containing a
`std::vector<std::string>`; missing or incorrectly typed claims deny access.
There are no implicit string conversions, wildcards, tenant inheritance,
or role hierarchies. The adapter establishing the SecurityContext **must**
verify any incoming permissions claim; `PrincipalBuilder` and
`SecurityContext::forPrincipal` do not validate credentials or trust.

Authentication provider integration, controlled request binding, HTTP
401/403 and redirect policy, and AuthSample updates belong to subsequent
Stage D increments. Existing session authentication continues to behave as
before. Do not use the current identity snapshot as proof of current
permissions for long-running Jobs: reauthorize at the operation boundary.
