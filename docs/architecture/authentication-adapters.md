# Authentication adapters

`<drogular/security/authentication_adapter.hpp>` defines an independent C++20
contract for verifying authentication data and constructing `SecurityContext`.
It does **not** parse JWTs, authorize actions or bind HTTP requests.

- `AuthenticationRequest` exposes non-owning header and session-value views.
  Implementations must keep those views valid while `authenticate()` runs.
  An adapter must not retain them, or record credentials in diagnostics/errors.
- `AuthenticationAdapter::authenticate()` returns `AuthenticationResult`, an
  `Expected<std::optional<SecurityContext>, AuthenticationError>`.
  `std::nullopt` means the adapter does **not** apply (no recognized credentials).
  A valid authenticated context means credentials were verified. An error
  indicates rejected/expired credentials, provider failure, or another failure.
- `AuthenticationPipeline` evaluates registered adapters in insertion order.
  The first verified identity wins. A rejection stops evaluation; **never**
  fall back to another adapter after invalid credentials. No applicable
  adapters results in `SecurityContext::anonymous()`.
- Returning an anonymous context as an adapter success is rejected. Exceptions
  are converted to `InternalError` without exposing exception messages.
- The chain must be assembled at application startup and treated as immutable
  while requests are being authenticated. Adapters should be thread-safe.

```cpp
class MyAdapter final : public drogular::security::AuthenticationAdapter {
public:
    drogular::security::AuthenticationResult authenticate(
        const drogular::security::AuthenticationRequest& request) const override;
};

drogular::security::AuthenticationPipeline pipeline;
pipeline.add(std::make_shared<MyAdapter>());
// D.3/D.4 will provide the HTTP/session facade and request binding.
```

`PrincipalBuilder` and `SecurityContext::forPrincipal()` only construct identity
values; neither validates credentials. Implementations of adapters MUST verify
credentials and trustworthy claims before returning an authenticated context.

This API does not change `AuthSupport`, `PageAuthSupport`, `ActionAuthSupport`,
`ActionContext` or `RequestContextBridge`. Session adapter, HTTP status mapping,
and AuthSample wiring are separate planned increments.

## Server-side session adapter

`<drogular/security/session_authentication_adapter.hpp>` adds an **opt-in**
`SessionAuthenticationAdapter` and a `SessionAuthenticationRequest` facade.
The facade takes an existing server-side `Session` resolved by `SessionStore`;
it does not trust or parse raw cookies. A missing session or missing `username`
returns not-applicable, while a present but invalid identity fails closed.

```cpp
// An HTTP integration must first resolve the session ID using SessionStore.
auto request = drogular::security::SessionAuthenticationRequest{existingSession};
auto pipeline = drogular::security::AuthenticationPipeline{};
pipeline.add(std::make_shared<drogular::security::SessionAuthenticationAdapter>(
    "my.application"));
auto identity = pipeline.authenticate(request);
```

The default subject key is `username` to match the existing `AuthSupport`
contract. An explicit, stable issuer is required to namespace identities.
The adapter deliberately **does not import session roles or permissions**:
those require a separately reviewed trust and authorization policy.
The resulting `SecurityContext` carries the method `session` and remains
valid after the original session/request is destroyed.

The facade caches string values for the duration of synchronous authentication
so its returned `string_view`s do not dangle. Do not reuse or concurrently
share one facade across requests. No existing `AuthSupport`, `PageAuthSupport`
or `ActionAuthSupport` behavior is changed. Automatic HTTP binding, failure
responses and redirects are addressed in next Stage.
