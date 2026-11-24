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
