# JobsDemo (Drogular 0.26)

A development-only example of the transport-neutral Job Runtime integrated into
Drogular Application and the existing Developer Tools diagnostics.

From the repository root, build the `jobs_demo` CMake target and run:

```bash
./build/examples/jobs_demo/jobs_demo --port 8084
```

The CLI follows PortalDemo: `--port <1..65535>` / `-p` overrides the default
8084, and `--help` / `-h` prints usage. Run from the repository root because
assets and templates are located using repository-relative paths.

To run in Docker from the repository root:

```bash
docker build -f Dockerfile.test -t drogular-tests .
docker build -f Dockerfile.jobs-demo -t drogular-jobs-demo .
docker run --rm -p 127.0.0.1:8084:8084 drogular-jobs-demo
```

The Docker image reuses the pinned Drogon dependency environment used by the
Linux test container. Only bind the port to loopback: these demo endpoints do
not provide authentication or CSRF protection. The image can override the
command (e.g. `jobs_demo --port 8084`), but update port mapping accordingly.

The HTML is maintained in `templates/index.html`; `main.cpp` configures the
application, while `routes/jobs_demo_routes.cpp` contains demo HTTP handlers.
See [browser-client reuse audit](docs/job-client-api-audit.md) and
[shared JobClient architecture](../../docs/architecture/job-client.md) for the
client/server boundary.

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

**Limitations:** Jobs and results live in process memory. Runtime retains up to
256 terminal jobs by default and evicts older terminal snapshots; active jobs
are not evicted. There is no durable persistence across server restarts.
HTTP/SSE disconnect and shutdown stress tests remain release-qualification
checks.

**Live events:** JobsDemo opens at most two job-scoped EventSource connections
at once. Remaining jobs use periodic polling (every 2.5 seconds). Each stream
is closed explicitly after a terminal event; the server sends HTTP 204 for
already-finished jobs, and releases its connection lease when the stream worker
exits. Polling also catches completion if an SSE event was missed. After all
jobs finish, `activeSseConnections` and `activeSubscriptions` should return to
zero (allow up to one heartbeat interval for a disconnected client).

## Job Registry and Developer Tools

The independent C++ `JobRegistry` provides a catalog of job definitions,
typed parameter constraints, and a validated `launch()` operation. JobsDemo
registers `demo.batch` and `demo.failure` in `main.cpp` and publishes the
catalog through `JobRegistryContributor` as a Developer Tools section named
**Available Jobs**. Runtime execution remains in `JobRuntime`.

The built-in Developer Tools renderer generates input controls from the registry
metadata. Launch is **explicitly opt-in** via a supplied launch endpoint. This
example permits it only from loopback requests with the Developer Tools header;
**this is not sufficient production authentication or CSRF protection**. Never
expose JobsDemo to untrusted networks. A production application must enforce
session authentication, per-job authorization, origin and CSRF checks.

`App::jobClient()` serves Drogular's shared, DOM-free ES module at
`/__drogular/assets/job-client.js`. JobsDemo imports it directly; there is no
local copy. `jobs.js` owns presentation, polling, reconnection and UI lifecycle.
Other applications opt in with `app.jobClient()` and import `JobClient` from
that path. Each application must provide authenticated, authorized job HTTP
routes; enabling the client module never exposes server-side endpoints.

The demo-specific `POST /api/jobs/registry/launch` endpoint accepts form fields
`type` and `seconds`, returns a 201 Job snapshot, and is restricted to local
requests with `X-Drogular-Developer-Tools: 1`. The existing `/api/jobs/create`
route remains for backwards compatibility and now launches the same registered
factories. Browse the **Available Jobs** section in `/__drogular` to use the
auto-generated launch form.

## Resource allocation demonstration

The **Memory allocation** control offers `0`, `0.5`, `1`, and `2 MiB`
(`memoryKiB=0|512|1024|2048` on the demo create route). Both registered
job types accept `memoryKiB` (integer, 0–2048) for Developer Tools launches.
The running task touches one byte per 4096-byte page and retains the vector
until completion, failure, or cooperative cancellation. Each progress update
includes the requested amount, and a successful result includes a checksum.

Watch the independent Process CPU/RSS history in Developer Tools while running
multiple jobs. Physical RSS may remain elevated after the vector is freed:
allocators can retain reusable pages, so the chart is not a measurement of
per-job memory ownership. Do not expose these unauthenticated demo routes to
untrusted networks.

## Cooperative Pause / Resume

JobsDemo uses opt-in resumable tasks. A running Job yields at its next
cooperative checkpoint, frees the worker slot, and enters `Paused`. Resume
requeues it at the back of the bounded FIFO queue; it continues from
application-managed checkpoint state when a worker becomes available. If the
queue is full, Resume is rejected and the Job remains paused. Legacy
`JobRuntime::submit()` tasks instead retain the worker while paused for
backward compatibility. The Jobs view offers Pause, Resume and Cancel;
JSON endpoints `POST /api/jobs/pause?id=<id>` and
`POST /api/jobs/resume?id=<id>` are for this local demonstration only.
