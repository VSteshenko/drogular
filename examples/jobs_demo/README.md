# JobsDemo (Drogular 0.26)

A development-only example of the transport-neutral Job Runtime integrated into
Drogular Application and the existing Developer Tools diagnostics.

From the repository root, build the `jobs_demo` CMake target and run:

```bash
./build/examples/jobs_demo/jobs_demo
```

Open <http://localhost:8084>. Use **Run job** or **Run failing job** to see
queued/running/terminal states, progress, typed result, error code and cooperative
cancellation. Switch to **Diagnostics** to inspect Job Runtime and SSE counters,
or use <http://localhost:8084/__drogular> for the standard inspection UI.

The frontend listens to `/api/jobs/events?job=<id>` using native EventSource,
with automatic reconnection, `Last-Event-ID` replay and `job.reset` resync.
`GET /api/jobs` also supplies an occasional snapshot as a fallback.

| Method | Route | Description |
| --- | --- | --- |
| GET | `/api/jobs` | Job snapshots |
| POST | `/api/jobs/create?seconds=5&fail=0` | Bounded demo job |
| GET | `/api/jobs/detail?id=1` | Job snapshot |
| POST | `/api/jobs/cancel?id=1` | Request cancellation |
| GET | `/api/jobs/diagnostics` | Job diagnostics snapshot |
| GET | `/api/jobs/events?job=1` | Job-scoped SSE |

**Security:** This sample deliberately has no authentication, authorization, or
CSRF protection. Do not expose port 8084 outside a trusted local development
environment. Production applications must authenticate all endpoints, enforce
job-level ownership and protect state-changing requests against CSRF.

**Limitations:** Jobs live in process memory, and the demo retains completed
jobs until shutdown. Runtime persistence and retention policies are outside
Stage F. This example exercises the existing SSE adapter; load and disconnect
stress tests belong to Stage G.

**Live events:** JobsDemo opens at most two job-scoped EventSource connections
at once. Remaining jobs use periodic polling (every 2.5 seconds). Each stream
is closed explicitly after a terminal event; the server sends HTTP 204 for
already-finished jobs, and releases its connection lease when the stream worker
exits. Polling also catches completion if an SSE event was missed. After all
jobs finish, `activeSseConnections` and `activeSubscriptions` should return to
zero (allow up to one heartbeat interval for a disconnected client).
