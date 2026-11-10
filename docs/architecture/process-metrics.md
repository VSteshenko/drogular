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
CPU percentage is null until two valid sampling points exist. Sampling is
on-demand and serialized, without background threads. Short sampling intervals
are noisy; space monitoring calls according to the desired resolution.

Linux uses `/proc/self/status`, `/proc/self/stat` and `/proc/uptime`; macOS uses
Mach task info, `task_threads`, `sysctl`, and `getrusage`. The process peak on
macOS is sourced from `ru_maxrss` (bytes), while on Linux the peak uses `VmHWM`
(KiB), falling back to `ru_maxrss` (KiB). Memory figures are converted to bytes.
A platform failing to provide RSS reports `available=false`. Unsupported
platforms return an unavailable snapshot, without crashing inspection.

`drogular::App` owns a provider available through `app.processMetrics()` and
exposes its snapshot as the independent `process` / `drogular.process` section
of the existing `ApplicationInspection` contract. Developer Tools renders this
section separately from Jobs. The developer inspection UI uses a manual Refresh
button to sample again; there is no background polling or new HTTP endpoint.

This contract also provides a starting point for the proposed 0.28 Metrics
Exporter. Windows collection is planned separately in 0.29.
