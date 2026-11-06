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


## Results, errors, and timings

`JobContext::setResult(value)` stores a copyable C++ value in the Job snapshot.
Read it with `snapshot.resultAs<T>()` after `Completed`; a mismatched type
returns `nullptr`. The result is cleared on failure or cancellation. The
runtime does not serialize arbitrary C++ results: an HTTP/Application adapter
must deliberately choose its own public representation and access policy.

```cpp
const auto id = jobs.submit("Count", [](drogular::JobContext& job) {
    job.reportProgress(50, "Counting");
    job.setResult(42);
});
// After the Job reaches Completed:
if (auto snapshot = jobs.find(id); snapshot &&
    snapshot->status == drogular::JobStatus::Completed) {
    if (const auto* count = snapshot->resultAs<int>()) {
        // Use *count.
    }
}
```

Throw `drogular::JobFailure("invalid_input", "Missing input")` for a stable
machine-readable `failure.code`. Other `std::exception` instances use code
`exception`; unknown exceptions use `unknown_exception`. The existing `error`
string remains for backwards compatibility. Failed or cancelled Jobs never
publish partial results. `queueDuration` is populated when a Job begins;
`executionDuration` is populated when it terminates after beginning. Cancelled
queued Jobs have neither duration. Both durations use milliseconds.

The `std::any` result is held in process memory and retained for the runtime
lifetime. Avoid storing large payloads or secrets without an appropriate
retention and authorization policy.

## Remaining 0.26 work

Application adapter, SSE transport and replay,
retention policy, JobsDemo, extended timings and concurrency diagnostics, and
stress/integration tests are **not yet implemented**.
