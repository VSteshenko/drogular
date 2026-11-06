# Job Runtime (0.26 development)

The process-local `drogular::JobRuntime` is independent of Drogon, HTTP, and
Drogular Application. It owns a bounded FIFO queue and a fixed worker pool.
A Job is submitted with `submit(name, callable)`, inspected with `find(id)` or
`list()`, and cooperatively cancelled with `requestCancel(id)`.

```cpp
#include <drogular/job_runtime.hpp>

// Keep the runtime alive until all submitted tasks have finished.
drogular::JobRuntime jobs(2, 128);
auto id = jobs.submit("Index", [](drogular::JobContext& job) {
    job.reportProgress(25, "Reading inputs");
    job.throwIfCancellationRequested();
    // Perform bounded work; check for cancellation regularly.
    job.reportProgress(100, "Indexed");
});
```

## Lifecycle and concurrency

- `Queued` -> `Running` -> `Completed` / `Failed` / `Cancelled`.
- Queued cancellation is immediate; running cancellation is a request and only
  becomes `Cancelled` when the task observes it. A task that catches and
  suppresses `JobCancelled` cannot be forcibly terminated.
- Throwing an exception fails the Job, and the error message is inspectable.
- Progress is an integer from 0 through 100; completed Jobs have progress 100.
- All returned snapshots are copies; lookup and progress reporting are
  synchronized. IDs are unique within one runtime instance, not globally.
- Shutdown rejects new submissions, cancels queued work, requests cancellation
  for running work, then joins workers. A blocking/non-cooperative task can
  delay shutdown indefinitely. Do not call shutdown from one of its own jobs.
- The runtime retains Job snapshots until it is destroyed. Retention/eviction
  policies must be added before long-lived high-volume production use.
- `RuntimeDiagnostics` is an optional, non-owning pointer. It must outlive the
  JobRuntime and exposes aggregate Job counters in the existing Developer Tools
  `jobs` section. This is not a persistent event log.

## Remaining 0.26 work

Application adapter, structured typed results, SSE transport and replay,
retention policy, JobsDemo, extended timings and concurrency diagnostics, and
stress/integration tests are **not yet implemented**.
