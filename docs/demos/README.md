# Demo Applications

Demo applications showcase complete production-oriented projects built with Drogular.

Current demos:

- Portal Demo
- System Monitor PWA
- Mini Store

Planned demos:

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

## Mini Store

Mini Store is the second reference application for Drogular and demonstrates a localized
Storefront and Staff experience in one application. It combines external EN/DE translation
catalogs, product/category data, search/filter/sort flows, a session-backed cart and checkout,
order placement and stock validation, Staff CRUD workflows, and order lifecycle transitions.

The example also exercises Drogular Interactions, canonical/history navigation, themes, and
PWA/offline behavior across both public and Staff surfaces. Its application-level regression
tests complement the domain/support tests and protect the main rendering and workflow contracts.
