# Server-driven fragments with Drogular Interactions

Use this pattern when a page should update part of its HTML without moving domain
rendering, localization, filtering, or presentation logic into a client-side
application.

Drogular Interactions provides the request lifecycle; the server continues to
return ordinary HTML fragments.

---

## 1. Enable the runtime

```cpp
#include <drogular/app.hpp>

int main() {
    drogular::App app;

    app.interactions()
       .templateRoot("templates");

    // Register pages/actions/components...
    app.run(8080);
}
```

Load the runtime in the application layout:

```html
<script src="/__drogular/assets/interactions.js" defer></script>
```

Drogular does not inject this tag automatically.

---

## 2. Declare a fragment request

```html
<section
    id="activity"
    dg-get="/fragments/activity"
    dg-target="[data-activity-list]"
    dg-trigger="load, every 5s">

    <div data-activity-list></div>
</section>
```

The browser requests `/fragments/activity`, expects HTML, and replaces the target.
The same request runs immediately and then every five seconds.

Keep filtering and formatting on the server. If named controls are inside the
interaction root, their current values are added to the request query string.

```html
<section
    dg-get="/fragments/activity"
    dg-target="[data-activity-list]"
    dg-trigger="load, input delay:250ms">

    <input name="search" type="search">
    <div data-activity-list></div>
</section>
```

---

## 3. Return semantic fragment state

For a successful non-empty result, return ordinary HTML.

For an empty result, mark the fragment:

```html
<div data-dg-empty>
    No matching activity.
</div>
```

The interaction root receives one of:

```text
dg-loading
dg-ready
dg-empty
dg-error
```

and the corresponding `data-dg-state` value. This gives application CSS a generic
state contract without teaching the runtime about the domain.

---

## 4. Add resilient polling when needed

For live data, connection state can remain declarative:

```html
<section
    id="live-data"
    dg-get="/fragments/live"
    dg-trigger="load, every 2s"
    dg-failure-limit="3"
    dg-pause-on-failure
    dg-connection="#live-status"
    dg-retry="#live-retry">

    <div data-live-content></div>
</section>

<div
    id="live-status"
    data-dg-label-connecting="Connecting"
    data-dg-label-live="Live"
    data-dg-label-reconnecting="Reconnecting"
    data-dg-label-offline="Offline">
    <span data-dg-connection-label></span>
    <span data-dg-connection-detail hidden></span>
</div>

<button id="live-retry" dg-resume="#live-data" hidden>
    Retry
</button>
```

After three consecutive failures, polling pauses and the connection state becomes
`offline`. Retry resumes polling and performs an immediate request.

The server can report a successful but degraded result such as stale cached data:

```html
<div
    data-dg-connection-state="stale"
    data-dg-connection-label="Stale"
    data-dg-connection-detail="Showing the last successful snapshot">
    ...
</div>
```

---

## 5. Add Drogular UI only if useful

Drogular Interactions does not require the framework stylesheet. If the application
wants the shared primitives, enable it independently:

```cpp
app.ui().interactions();
```

and load:

```html
<link rel="stylesheet" href="/__drogular/assets/ui.css">
```

For example:

```html
<div id="live-status" class="dg-status">
    <span data-dg-connection-label></span>
    <span data-dg-connection-detail hidden></span>
</div>
```

The `dg-status` primitive maps the standard connection states to semantic styling.
Application layout and branding remain application-owned.

---

## 6. Preserve native disclosure state

When a refreshed fragment contains native `<details>` elements, give stable items a
key:

```html
<details data-dg-preserve-key="device-42">
    ...
</details>
```

The runtime restores the open/closed state after replacement. This is useful for
server-rendered hardware inventories, diagnostics panels, and other periodically
refreshed lists.

---

## Design guidance

Prefer server-driven fragments when the server already owns the data transformation
and presentation model and the browser only needs orchestration and replacement.

Keep JSON endpoints when they are useful machine-readable contracts; an HTML fragment
endpoint can exist alongside them. System Monitor PWA follows this pattern for live
system, process, and hardware views.

See the complete [Drogular Interactions reference](../reference/interactions/README.md)
and the [System Monitor PWA](../../examples/system_monitor_pwa/README.md).