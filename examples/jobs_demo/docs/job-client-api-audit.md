# JobsDemo browser-client reuse audit (Drogular 0.26)

## Current boundary

The transport-neutral C++ `JobRuntime` and `JobEventStream` belong to the core.
`registerJobSseEndpoint()` is a Drogon adapter. `JobsDemo` owns demo-only
routes and presentation. The `jobs.js` file currently mixes API calls, SSE
recovery, caching state, DOM rendering, button actions, and diagnostics cards.
Copying it into another application would couple that application to the demo
route names, markup, status messages, and security assumptions.

## Candidate reusable browser API

Introduce a separately shipped, framework-owned ES module (proposed:
`drogular/jobs-client.js`) with *no DOM dependencies*. It should expose:

| Responsibility | Proposed surface | Constraints |
| --- | --- | --- |
| HTTP transport | `JobClient({ baseUrl, fetch })` | Configurable routes; do not assume demo `/create` shape |
| Job reads | `list()`, `get(id)` | Normalized snapshots and explicit HTTP errors |
| Job actions | `cancel(id)` | Respect application's CSRF/auth policy |
| Events | `watch(id, listener, options)` | Return disposable handle; terminal state closes it |
| Recovery | Replay, reset, bounded reconnect backoff | Resync using GET; avoid unlimited per-job streams |
| Subscription budget | Per-origin/global stream limit | Preserve room for ordinary HTTP requests |
| Diagnostics | `getDiagnostics()` optional | No assumption that diagnostics are public |

Separate the HTTP endpoint contract from the browser client. Production
applications need authenticated, authorized job-scoped endpoints; **do not**
expose the demo's unauthenticated routes as default framework routes. SSE
`Last-Event-ID` replay should be transparent to callers, but `job.reset`
requires a fresh snapshot. Consumers must receive lifecycle and error events
rather than translated DOM strings.

## Keep in JobsDemo

`render()`, progress bars, tab selection, notices, the demo's bounded
`seconds`/`fail` submission form, formatting of metrics, HTML/CSS and polling
frequency for this demonstration remain example-specific.

## Recommended implementation sequence

1. Define a versioned HTTP/job-event contract and test it independently of UI.
2. Extract a DOM-free JobClient from `jobs.js`, with dependency-injected fetch,
   EventSource factory, event callbacks and deterministic test coverage.
3. Serve the shared ES module using the same resource delivery approach as
   other Drogular client assets; keep `jobs.js` only as the demo view/controller.
4. Add a second small consumer test, including authorization-denied, network
   failure, terminal event, reset/replay and reconnect scenarios.

**Decision:** do not prematurely publish the current demo JavaScript as stable
API. The desired API boundary is the transport/state layer, not the example UI.
