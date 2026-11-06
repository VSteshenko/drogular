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

## Process-local events and SSE framing

`JobRuntime::events()` exposes a transport-neutral `JobEventStream` (see
`<drogular/job_events.hpp>`). It emits `Queued`, `Started`, `Progress`,
`CancellationRequested`, and the terminal `Completed`, `Failed`, `Cancelled`
events, each with a monotonic **runtime-local** sequence number and a copy of
its Job snapshot. Subscribers can filter by Job ID and replay from a cursor:

```cpp
#include <drogular/job_events.hpp>
#include <drogular/job_sse.hpp>

auto subscription = jobs.events().subscribe(jobId, lastEventId);
for (auto batch = subscription.waitFor(std::chrono::seconds(15));
     !batch.events.empty() || batch.gap;
     batch = subscription.waitFor(std::chrono::seconds(15))) 
{
    if (batch.gap) {
        // Send encodeJobSseReset(), then fetch an authorized snapshot.
    }
    for (const auto& event : batch.events) {
        auto frame = drogular::encodeJobSse(event);
        // Send frame on the HTTP stream.
    }
}
```

`subscribe(..., std::nullopt)` listens only for new events; a specified cursor
replays events with a higher sequence. The replay ring and per-subscriber queue
are bounded (defaults: 256 each). When replay is unavailable, a cursor is in
the future, or a slow subscriber overflows, `JobEventBatch::gap` is set. **Do
not assume event delivery is lossless**; send `encodeJobSseReset()` and obtain
a new snapshot with `find()` or `list()`. For a continuous SSE connection,
respond with `Content-Type: text/event-stream`, `Cache-Control: no-cache`,
write frames as they arrive, use `encodeJobSseHeartbeat()` on idle timeouts,
and honor `Last-Event-ID` for reconnects. Event IDs are not durable across
runtime restarts, so reconnect handling must resync if the process changed.

The framing helper deliberately serializes only basic metadata, progress and
errors; opaque `std::any` results are **never** exposed automatically. The HTTP
controller must apply authorization, isolate users' jobs, manage disconnects,
and avoid blocking Drogon event-loop threads with `waitFor()`. A concrete
Drogon streaming controller is **not included yet**; that integration is part
of the upcoming Application adapter and JobsDemo stage.

The event stream is process-local and is not a durable audit trail. Subscribers
are RAII handles. Consuming a batch clears its overflow flag; an SSE adapter
must process the gap before applying subsequent events. Event publication is
bounded and does not call arbitrary user callbacks on worker threads.

## Remaining 0.26 work

Application adapter, SSE transport and replay,
retention policy, JobsDemo, extended timings and concurrency diagnostics, and
stress/integration tests are **not yet implemented**.
