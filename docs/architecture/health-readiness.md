# Health and Readiness

Health checks are independent from Job Runtime and Process Metrics. They report
whether an application can serve traffic, not whether individual jobs are idle.

## Enable endpoints

```cpp
#include <drogular/app.hpp>

app.health().add("database", [] {
    return drogular::HealthCheckResult{drogular::HealthStatus::Healthy, {}};
}, std::chrono::seconds(2));
app.enableHealthEndpoints();
```

Both endpoints are explicitly opt-in:

- `GET /health/live`: `{ "status": "healthy" }` with HTTP 200 when the
  HTTP server can respond. It never runs dependency checks.
- `GET /health/ready`: JSON `status` and `checks` array, HTTP 200 only if
  all checks are healthy; HTTP 503 for degraded or unhealthy outcomes.
  No checks registered means ready. Add at least one dependency check if an
  application's readiness depends on an external service.

Readiness checks have a configurable timeout (default two seconds), catch
exceptions and report unhealthy without exposing exception text in the public
HTTP response. The HTTP event loop does not execute readiness probes.
When a previous run of an individual check is still pending, a new probe
reports that check as unhealthy instead of launching another copy. This limits
stuck workers to at most one per registered check. A timed-out callback is not
forcibly terminated: implement network timeouts within the check itself as well.
Checks should be thread-safe; concurrent readiness requests can call distinct
checks simultaneously.

There is no automatic database, cache or Job Runtime check. Applications
register only critical dependencies. Do not return credentials or sensitive
information in check names; endpoints are intended for internal monitoring and
should be protected by network-level access policy if deployed publicly.

`HealthStatus::Degraded` is represented explicitly but fails readiness by
default (503). This conservative mapping ensures load balancers do not route
new requests to a partially ready instance.
