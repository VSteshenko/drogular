# Process Metrics (Drogular 0.26 Stage F)

`drogular::ProcessMetricsProvider` is independent of Job Runtime, HTTP, Drogon,
application services, and Developer Tools. It samples **the entire server process**.
No values in this API represent CPU or memory attributable to an individual Job.

```cpp
#include <drogular/process_metrics.hpp>

drogular::ProcessMetricsProvider metrics;
auto first = metrics.snapshot();   // CPU percentage unavailable initially
// ... some time later ...
auto second = metrics.snapshot();  // CPU percentage over the interval
```

`ProcessMetricsSnapshot` contains `available`, nullable `cpuPercent`, `residentBytes`,
`peakResidentBytes`, `threadCount`, `uptimeMs`, and `cpuTimeMs`. CPU percentage
uses **one fully utilized logical core = 100%** (multi-threaded loads may exceed
100%). `cpuTimeMs` is accumulated user plus system time; it is not wall time.
CPU percentage is null until two valid sampling points exist. The low-level
provider remains on-demand and serialized; the independent history sampler
collects snapshots every two seconds once the application starts serving.

Linux uses `/proc/self/status`, `/proc/self/stat` and `/proc/uptime`; macOS uses
Mach task info, `task_threads`, `sysctl`, and `getrusage`. The process peak on
macOS is sourced from `ru_maxrss` (bytes), while on Linux the peak uses `VmHWM`
(KiB), falling back to `ru_maxrss` (KiB). Memory figures are converted to bytes.
A platform failing to provide RSS reports `available=false`. Unsupported
platforms return an unavailable snapshot, without crashing inspection.

`drogular::App` owns a provider available through `app.processMetrics()` and
exposes its snapshot as the independent `process` / `drogular.process` section
of the existing `ApplicationInspection` contract. Developer Tools renders this
section separately from Jobs. `ProcessMetricsHistory`, owned by `App`, uses a single background sampler
thread, a 2-second interval and a 150-record ring buffer (up to five minutes).
`app.processMetricsHistory().snapshot()` returns a thread-safe copy with
relative `elapsedMs` and CPU/RSS values. Recording is in memory only; it stops
when `App::run()` returns. `ApplicationInspection` includes a bounded `history`
array in the existing `process` section, without creating a separate HTTP API.
Developer Tools renders CPU and RSS SVG charts and refreshes only the Process
section every two seconds, preserving inputs in other Developer Tools panels.
The first CPU sample is null; gaps must not be represented as zero. Samples
are process-wide and must never be attributed to individual Jobs.

This contract also provides a starting point for the proposed 0.28 Metrics
Exporter. Windows collection is planned separately in 0.29.
