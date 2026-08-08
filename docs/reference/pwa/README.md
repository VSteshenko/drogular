# PWA

Drogular provides small server-side helpers for Progressive Web App metadata, service-worker registration, local browser persistence, offline-status UI, and root-level service-worker delivery.

The PWA API is intentionally split into two layers:

```text
Page rendering
    │
    ├── PwaOptions
    ├── PwaHtml
    ├── PwaScripts
    └── PwaPageSupport

Application routing
    │
    ├── App::staticFiles()
    ├── App::serviceWorker()
    └── App::offlinePage()
```

`PwaPageSupport` prepares template values. `App` is responsible for exposing the resources and routes those values refer to.

---

## Types

- [`PwaOptions`](pwa-options.md) — paths and metadata used when generating PWA markup.
- [`PwaHtml`](pwa-html.md) — generates manifest, favicon, and theme-color markup.
- [`PwaScripts`](pwa-scripts.md) — generates service-worker, persistence, and online/offline scripts.
- [`PwaPageSupport`](../pages/pwa-page-support.md) — writes standard PWA snippets into a `RenderContext`.

---

## Typical Flow

```text
App::staticFiles()
        │
        ├──────────────► manifest / icons / assets
        │
App::serviceWorker()
        │
        └──────────────► /service-worker.js

Page::onInit()
        │
        ▼
PwaPageSupport::apply()
        │
        ├── manifestLink
        ├── faviconLink
        ├── themeColorMeta
        └── serviceWorkerRegistration
        │
        ▼
Template
```

An offline fallback page is optional. `App::offlinePage()` only registers the page route; the service worker must explicitly use that route in its own fetch/fallback logic.

---

## Example

TodoPWA configures the resources at application startup:

```cpp
app.staticFiles(
    "/assets",
    "examples/todo_pwa/public"
)
.serviceWorker(
    "examples/todo_pwa/public/service-worker.js"
);

app.offlinePage<TodoPwaOfflinePage>();
```

The offline page then adds the standard PWA template values:

```cpp
void onInit(
    drogular::RenderContext& context
) override
{
    drogular::PwaPageSupport::apply(context);
}
```

---

## Important Behavior

- `App::serviceWorker()` always serves the configured file at `/service-worker.js`.
- `PwaOptions::serviceWorkerPath` only changes the URL embedded in the registration script; it does not create a matching route.
- `App::offlinePage()` registers a normal Drogular page route. It does not modify or generate service-worker fetch logic.
- `PwaHtml` and `PwaScripts` interpolate supplied strings directly and do not perform HTML or JavaScript escaping. Pass trusted application-controlled values.
- `PwaScripts::inputPersistence()` and `offlineStatus()` look up their target elements immediately when the generated script executes. Place those scripts after the relevant markup or otherwise ensure the elements already exist.

---

## See Also

### API Reference

- [`App`](../application/app.md)
- [`PageSupport`](../pages/page-support.md)
- [`PwaPageSupport`](../pages/pwa-page-support.md)
- [`RenderContext`](../rendering/render-context.md)
- [`Router`](../routing/router.md)
