# Drogular 0.26.0 — Release Notes

**Version:** 0.26.0

## Highlights

- **Independent Job Runtime:** bounded FIFO queue, fixed worker pool, monotonic
  process-local IDs, progress, typed results, structured errors and cancellation.
- **Cooperative Pause/Resume:** legacy `submit()` pauses keep their worker;
  opt-in `submitResumable()` yields at checkpoints, frees the worker and requeues
  on Resume. Resumable callbacks must preserve their own checkpoint state.
- **Job Registry:** validated typed parameters, fixed choices and explicit
  application-controlled launch integration in Developer Tools.
- **Job Events and SSE:** replay with gap detection, job-scoped streaming,
  reconnection support, resource limits and managed forwarding threads.
- **Job Diagnostics:** queue/worker gauges and peaks, duration averages/maxima,
  pause/requeue, failure/cancel and SSE connection counters.
- **Process Metrics:** independent macOS/Linux CPU, RSS, peak RSS, threads and
  uptime sampling; bounded five-minute resource history and Developer Tools charts.
- **Reusable JobClient:** served as the opt-in ES module
  `/__drogular/assets/job-client.js`, with application-owned authorization.
- **JobsDemo:** parameterized jobs, memory-allocation demonstration, pause,
  resume, cancellation, SSE/polling fallback, diagnostics, Docker and CLI.

## Compatibility and behavior

The previous `JobRuntime::submit()` API preserves its cooperative blocking
pause semantics; `submitResumable()` is a separate opt-in execution model.
Resumable callbacks restart after yielding and must use saved, idempotent
checkpoint state. RSS reflects whole-process usage, not the memory attributed
to an individual job. HTTP/SSE adapters and client resources require explicit
application opt-in and do not establish access control by themselves.

The runtime retains at most 256 terminal jobs by default, unless configured
otherwise. Active jobs are not evicted; job data is not durable across process
restarts. Recent SSE events may temporarily hold copies of terminal snapshots.

## Platform and dependencies

The process metrics collectors support macOS and Linux. JobsDemo uses Drogon
asynchronous streaming; Linux dependency builds must supply a compatible
Drogon version rather than Ubuntu 24.04's older `libdrogon-dev` package.
Windows platform support remains planned for 0.29 (MSVC/Windows CI; MinGW
outside initial scope).

## Testing and release qualification

The final macOS/Linux test suites and JobsDemo scenarios were checked by the
project maintainer before preparing this release. The release process should
still verify that the published tag points to the exact tested commit.

## Security and limitations

JobsDemo is a **development-only** example and intentionally lacks production
authentication and CSRF safeguards. Do not expose it to an untrusted network.
Production integrations must enforce application and job-level authorization,
rate/resource limits and CSRF protection for state-changing requests. The
Runtime is process-local; durable jobs, distributed scheduling and external
metrics export are outside the scope of this release.

## Documentation

- [Job Runtime](docs/architecture/job-runtime.md)
- [Job Registry](docs/architecture/job-registry.md)
- [Browser JobClient](docs/architecture/job-client.md)
- [Process Metrics](docs/architecture/process-metrics.md)
- [JobsDemo](examples/jobs_demo/README.md)

## Roadmap

0.27: Health/readiness and authentication Principal extensions.
0.28: Audit hooks and Metrics Exporter.
0.29: Windows x64/MSVC support, Windows CI and cross-platform hardening.
