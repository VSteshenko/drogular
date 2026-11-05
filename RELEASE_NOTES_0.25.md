# Drogular 0.25 — Release Notes

Drogular 0.25 extends the 0.24 file-based translation system to browser/offline clients, introduces reusable safe-return 
navigation, and makes PortalDemo dashboard tasks first-class mutable domain objects.

## Client localization and offline support

- `ClientLocalizationCatalog` derives a manifest and one JSON translation resource per locale from an existing file-backed catalog. Manifest and per-locale content versions are deterministic.
- `App::clientLocalization()` exposes `/__drogular/assets/localization.js`; applications explicitly publish their manifest and locale resource endpoints.
- Browser API: `drogularLocalization.configure({manifestUrl})`, `ready(locale)`, `setLocale(locale)`, `translate(key)`, `loadManifest()`, and `loadResource(locale)`.
- Translation fallback remains requested locale → configured default locale → key. Versioned resources use a separate `drogular-localization-v1` Cache Storage namespace and network-to-cache fallback.
- PortalDemo serves `/assets/localization/manifest.json` and `/assets/localization/{locale}` from its `FileTranslationProvider`, and uses the runtime for its offline read UI instead of embedding a second translation table in HTML.
- PortalDemo Service Worker keeps unrelated caches intact on activation and precaches the framework localization script. Cached locale resources are not coupled to the application shell cache version.

## Navigation

- `NavigationContext` provides safe local target validation, fallback resolution, and encoded `returnUrl` propagation.
- PortalDemo and Mini Store use this shared contract for return navigation instead of separate URL validation rules.

## PortalDemo task dashboard

- `PortalTask`, `PortalTaskStatus`, `PortalTaskPriority`, and `PortalTaskProvider` separate tasks from project status.
- A unified `DemoDataset` holds initial PortalDemo sample records, including tasks. The in-memory task provider supports independent state mutations.
- Dashboard task status and priority can be updated through POST actions and server-rendered interaction fragments. Completion removes a task from the current list; priority changes affect ordering. Mutation controls are restricted to authorized users.

## Compatibility and migration notes

- Client localization is opt-in: register `app.clientLocalization()`, publish your own manifest/resource routes and configure the runtime in the browser. Existing server-only translation providers remain supported.
- Offline localization does not make arbitrary uncached pages or languages available without a network connection. Cache requested languages while online, and keep application authentication/session semantics in mind.
- `NavigationContext` is an optional application-level helper, not a replacement for route registration or access-control checks.
- PortalDemo data and task actions are example application APIs, not core framework task/job APIs. Framework-level background Jobs remain planned for 0.26.

## Testing

Localization catalog/runtime, navigation, PortalDemo actions and Service Worker lifecycle have regression tests. Run 
the complete CTest suite and verify the offline/language-switching flows in target browsers for release qualification.

## Documentation

See [client localization reference](docs/reference/localization/client-localization.md), [localization cookbook](docs/cookbook/localization.md), [Offline Read Models](docs/reference/offline/README.md), and [NavigationContext](docs/reference/routing/navigation-context.md).