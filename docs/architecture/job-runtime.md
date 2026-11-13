# Job Runtime (0.26 development)

The process-local `drogular::JobRuntime` is independent of Drogon, HTTP, and
Drogular Application. It owns a bounded FIFO queue and a fixed worker pool.
A Job is submitted with `submit(name, callable)`, inspected with `find(id)` or
`list()`, cooperatively cancelled with `requestCancel(id)`, and paused or resumed with
`requestPause(id)` / `requestResume(id)`.

```cpp
#include <drogular/job_runtime.hpp>

// Keep the runtime alive until all submitted tasks have finished.
drogular::JobRuntime jobs(2, 128);
auto id = jobs.submit("Index", [](drogular::JobContext& job) {
    job.reportProgress(25, "Reading inputs");
    job.waitIfPaused();
    // Perform bounded work; check for pause/cancellation regularly.
    job.reportProgress(100, "Indexed");
});
```

## Lifecycle and concurrency

- `Queued` -> `Running` -> `Completed` / `Failed` / `Cancelled`, with an
  intermediate `Paused` state for running tasks at cooperative checkpoints.
- `requestPause()` records `pauseRequested` and emits `job.pause-requested`;
  it does not forcibly suspend user code. A queued pause keeps the task queued
  and not eligible for worker dispatch until `requestResume()`. A running task
  enters `Paused` at `JobContext::waitIfPaused()`, or at `reportProgress()` or
  `setResult()` (which invoke the checkpoint).
- `requestResume()` clears the request and wakes waiting tasks. A paused worker
  **remains occupied**; there is no stack serialization/checkpoint-and-requeue.
  Running tasks that never reach a checkpoint cannot be paused promptly.
- `requestCancel()` and shutdown wake paused tasks so cancellation is observed.
  `pauseRequested` is independent of `cancellationRequested`. Terminal jobs are
  unaffected by Pause/Resume; unknown IDs return `false`.
- Pausing an already queued job does not count as `Paused` until it has started;
  pending requests are visible through `snapshot.pauseRequested`.
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


## Runtime and SSE diagnostics

Pass an optional `RuntimeDiagnostics*` when constructing `JobRuntime`. The
existing Developer Tools `drogular.jobs` section retains the original counters
and adds live `queued`, `running`, `workerCapacity` and `queueCapacity` gauges, cumulative
`queueWaitTotalMs` and `executionTotalMs`, `eventsPublished`, `eventGaps`,
`activeSubscriptions`, `activeSseConnections`, `sseConnectionsOpened`,
`sseConnectionsRejected`, and `sseConnectionsClosed`.

Queue and running gauges change on *state transitions*, not on polling.
Waiting times are counted when a job starts; execution times when a job
finishes. `queueWaitSamples` and `executionSamples` count precisely the
observations contributing to their respective totals. The diagnostics snapshot
provides rounded-down `queueWaitAverageMs` and `executionAverageMs`, plus
`queueWaitMaxMs`, `executionMaxMs`, `peakQueued`, and `peakRunning`. Current
worker utilization and queue saturation are represented in basis points
(`workerUtilizationBps`, `queueSaturationBps`; 10000 = 100%). A zero
capacity yields zero basis points; the UI displays these ratios as unavailable.
Pause diagnostics include `paused`, `pauseRequests`, `resumeRequests`,
`pauseTotalMs`, `pauseSamples` and `pauseAverageMs`. Time spent in Paused
is accumulated on wakeup; a currently paused interval is not yet included
in the aggregate total. `running` and worker utilization include paused tasks
because cooperative pause retains a worker.
Capacity gauges aggregate all active JobRuntime instances sharing diagnostics;
peaks are lifetime high-water marks for the aggregate, not per-runtime maxima.
The ratios are instantaneous gauges, not time-weighted utilization averages.


Metrics are non-transactional atomic snapshots and may be momentarily out of
sync while transitions are recorded concurrently. A queued cancellation does
not contribute to queue waiting time, because the job never started.

SSE connection gauges apply only to endpoints registered using
`registerJobSseEndpoint` and associated with a runtime that has diagnostics.
A connection is counted from admission until its streaming lease is released;
`Closed` counts lease releases including normal close and disconnects.
Stream `eventGaps` counts cursor gaps and subscriber-buffer overflows.
The diagnostics pointer must outlive the runtime and its SSE connections;
these metrics are aggregates, not a per-user activity log.

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
and avoid blocking Drogon event-loop threads with `waitFor()`. A Drogon adapter is provided below; applications must explicitly register
it and enforce per-job authorization.

The event stream is process-local and is not a durable audit trail. Subscribers
are RAII handles. Consuming a batch clears its overflow flag; an SSE adapter
must process the gap before applying subsequent events. Event publication is
bounded and does not call arbitrary user callbacks on worker threads.

## Drogon HTTP/SSE endpoint

The optional Drogon adapter lives in `<drogular/job_sse_drogon.hpp>` and is
registered explicitly by each application. It does **not** introduce a route
or publish task data by default. Pass a shared Runtime lifetime and a mandatory
per-job authorization check:

```cpp
#include <drogular/job_sse_drogon.hpp>

// jobs is a std::shared_ptr<drogular::JobRuntime>.
drogular::registerJobSseEndpoint(
    drogon::app(), "/api/jobs/events", jobs,
    [](const drogon::HttpRequestPtr& request, drogular::JobId jobId) {
        // Replace with a real principal/tenant ownership check.
        return authenticatedUserOwnsJob(request, jobId);
    });
```

Clients connect using `new EventSource('/api/jobs/events?job=123')` and
usually fetch the initial, **authorized** Job snapshot separately. Drogon
streams `text/event-stream` using `newAsyncStreamResponse`; a bounded number
of connection workers wait for events outside Drogon's event-loop threads.
The endpoint validates Job IDs, enforces authorization before exposing
existence, rejects unknown Jobs, reads `Last-Event-ID`, and sends heartbeats.
The default connection cap is 64; additional clients receive HTTP 503. A
replay or subscriber overflow sends `job.reset`, after which clients **must**
fetch an up-to-date authorized snapshot. SSE never serializes opaque results.

Important production constraints: this is a **process-local** adapter, not
cross-node event delivery. Its connection model dedicates a lightweight
worker thread per EventSource client (bounded by `maxConnections`). Connection
cleanup is detected by a failed socket send, so idle disconnections may
remain counted until the next heartbeat. A shared runtime must remain alive
while it is passed to the endpoint; this adapter captures its ownership.
Do not use an `authorize` callback that always returns true in production.

The transport integration is intentionally independent of `Drogular::App`:
Application routes and the future JobsDemo may register it without changing
the core Runtime. A full live-browser cross-platform integration test remains
a follow-up alongside JobsDemo.

## Remaining 0.26 work

Application convenience wiring, retention policy, JobsDemo, extended timings
and concurrency diagnostics, and full network stress/integration tests are
**not yet implemented**.
