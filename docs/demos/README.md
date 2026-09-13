# Demo Applications

Demo applications showcase complete production-oriented projects built with Drogular.

Current demos:

- Portal Demo
- System Monitor PWA

Planned demos:

- Store Demo
- Chat Demo

## Portal Demo template DSL

Portal Demo is also the reference application for Drogular's template expression
features. Its templates use application expression functions for localization
(`t()`), lexical `@let`/`@const` bindings, collection helpers, list membership,
and `@switch` for presentation-only state formatting. Page code keeps
request/domain preparation in C++ while translation keys and presentation
choices stay in templates.

## System Monitor PWA

System Monitor PWA is the reference application for server-rendered fragments and
progressively enhanced live UI. It uses the framework-level Drogular Interactions and
Drogular UI modules to demonstrate server-rendered system/process/hardware fragments,
declarative `dg-get` / `dg-target` / `dg-trigger` / `dg-resume` interactions, polling and
retry/offline behavior, macOS/Linux monitoring, Raspberry Pi hardware inspection,
SSH-backed remote monitoring, and PWA integration.

The example keeps application-specific layout and hardware presentation local while shared
interaction behavior and UI primitives come from `/__drogular/assets/`. Dashboard fragments
refresh independently according to their data cadence; the Hardware page uses a two-second
system connection probe alongside a 30-second aggregated `/fragments/hardware` refresh. No
application-specific JavaScript is required for either page.
See [`examples/system_monitor_pwa/README.md`](../../examples/system_monitor_pwa/README.md).