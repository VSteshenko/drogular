# HTTP authentication integration

The HTTP integration is **opt-in**. Existing `App::page()`, `App::action()` and
`App::get()` registrations remain public. Existing `AuthSupport`,
`PageAuthSupport` and `ActionAuthSupport` keep their legacy behavior.

## Configuration

```cpp
using drogular::security::HttpSecurityPolicy;

auto pipeline = std::make_shared<drogular::security::AuthenticationPipeline>();
pipeline->add(std::make_shared<drogular::security::SessionAuthenticationAdapter>(
    "my.application"));

app.authenticationPipeline(pipeline);
app.securePage<DashboardPage>("/dashboard", HttpSecurityPolicy::authenticated());
app.secureAction<UpdateAction>("/update", HttpSecurityPolicy::role("editor"));
app.secureGet<ReportAction>("/reports", HttpSecurityPolicy::permission("reports:read"));
```

Configure the pipeline and routes during startup, **before** `App::run()`.
Do not mutate the pipeline after starting the server. Both page and action
handlers receive `SecurityContext` through the existing request-scoped state;
`ActionContext::principal()` and `RenderContext::principal()` remain read-only.
The context is authenticated before the route handler or page renderer executes.

## Request credentials

The HTTP request facade reads request headers and resolves the `session_id`
cookie via the application `SessionStore`. Raw cookie content is never taken as
an identity. Session values are only available after that server-side lookup.
This integrates with the opt-in `SessionAuthenticationAdapter`.
No role or permission is inferred from arbitrary session fields.

## Status handling

* Public policy: allows anonymous requests.
* Authenticated policy: anonymous requests receive HTTP 401.
* Role and permission policy: anonymous requests receive 401; authenticated
  requests lacking the required role/permission receive HTTP 403.
* Invalid or expired credentials: HTTP 401, no adapter fallback.
* Provider unavailable: HTTP 503; internal adapter errors: HTTP 500.

Error responses use short generic messages and `Cache-Control: no-store`.
No login redirect is performed by this layer; existing AuthSample redirect
behavior remains in its legacy authentication helpers. This version does not
implement bearer-token-specific HTTP challenges or a customizable redirect
policy. Authentication failures and denial responses must never expose
credentials or provider error details.

The global pipeline, when configured, is evaluated on **all Drogular page and
action routes**, including public routes, allowing them to inspect the
identity. A rejected credential is never silently ignored even for a public
route. No pipeline means all routes start anonymous; an opt-in protected route
will therefore deny access. Static files, service workers, raw Drogon handlers,
and other special routes are not covered by this integration.

## Concurrency and trusted access

Adapter evaluation is synchronous in this iteration. Custom adapters must not
perform slow remote blocking calls on the Drogon event loop. A future release
may introduce asynchronous authentication providers. The `SecurityContext`
is an immutable identity snapshot, not a proof of current permissions for
long-running Jobs. Access must be reauthorized for sensitive operations.

This integration does not add JWT or OAuth2 verification; trusted custom
adapters must verify their own credentials before producing a Principal.

## Security review and regression coverage

`core_http_authentication_integration_tests.cpp` exercises the HTTP facade,
verified server-side session lookup, pipeline and route policy evaluation
**together** (without starting a network listener). It covers anonymous 401,
authenticated role/permission decisions, invalid-credential fail-closed behavior,
provider 503, untrusted cookie identifiers, request isolation and the generic
non-cacheable error responses. The existing unit tests separately cover
adapter result validation, exceptions, typed claims and request context lifetime.

**Security boundaries:** Never register a protected resource only as a public
`page()`/`action()`/`get()` route; use the `secure*` variants and explicitly
configure a trusted authentication pipeline. A `Principal` created by an
application-owned adapter is trusted only to the extent that adapter verifies
its credentials. Do not derive privileges from unverified HTTP headers or
untrusted session strings. The test-only header adapter is deliberately not
suitable for production. Static files, raw Drogon handlers, and other special
routes are outside this integration and require independent access controls.

**Current limitations:** HTTP authentication currently evaluates adapters
synchronously. Providers doing network or blocking I/O must not run on the
Drogon event loop. This release does not implement CSRF protections for
state-changing cookie-authenticated routes, session rotation/invalidation
policy, or bearer-token challenges; these must be addressed by the host
application or a later security extension before exposing sensitive endpoints.
This test file validates the integration helper, not live routing behavior;
full application/CI tests remain necessary for release qualification.
