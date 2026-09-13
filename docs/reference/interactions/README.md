# Drogular Interactions

Drogular Interactions is an optional browser runtime for progressively enhanced,
server-rendered HTML fragments.

It keeps request orchestration in the browser small and declarative while pages,
components, localization, filtering, and domain presentation remain server-owned.

Enable the runtime explicitly:

```cpp
app.interactions();
```

Then load the built-in script from the application:

```html
<script src="/__drogular/assets/interactions.js" defer></script>
```

`App::interactions()` is idempotent. Enabling Drogular Interactions does not enable
Drogular UI and does not inject a `<script>` tag automatically.

---

## Core request contract

Any element with `dg-get` becomes an interaction root when the runtime initializes.

```html
<section
    id="processes"
    dg-get="/fragments/processes"
    dg-target="[data-process-list]"
    dg-trigger="load, every 3s">
    <div data-process-list></div>
</section>
```

### `dg-get`

Specifies the URL used for a GET request. Named `input`, `select`, and `textarea`
controls inside the interaction root are serialized as query parameters. Disabled
controls and unchecked checkboxes/radio buttons are ignored.

### `dg-target`

Selects the element whose `innerHTML` is replaced by the returned fragment.

- omitted or `this`: replace the interaction root;
- otherwise the runtime first searches inside the interaction root, then the document.

### `dg-trigger`

Controls when requests run. Multiple triggers are comma-separated.

Supported forms:

- `load`
- `every Ns`, including decimal seconds such as `every 2.5s`
- `input`
- `input delay:Nms`
- `change`
- `change delay:Nms`

If `dg-trigger` is omitted, the default is `load`.

A `<form dg-get="...">` also refreshes on submit and prevents the normal browser
submission.

---

## Request state

The runtime exposes request lifecycle state on the interaction root:

```text
dg-loading
dg-ready
dg-empty
dg-error
```

The same state is published through `data-dg-state`. While loading, `aria-busy` is
set to `true`.

A returned fragment is treated as semantically empty when it contains an element
with `data-dg-empty`.

Drogular Interactions does not prescribe the visual presentation of these states.
Applications may style them directly or combine the runtime with Drogular UI.

---

## Polling and coordinated pause/resume

### `dg-poll-group`

Interaction roots with the same `dg-poll-group` value are paused and resumed as a
unit.

### `dg-failure-limit`

Sets the number of consecutive request failures before the interaction reaches the
offline state. The connection lifecycle is active only when `dg-connection` is also
present.

### `dg-pause-on-failure`

When the failure limit is reached, polling is stopped and the interaction root gets
`data-dg-paused="true"`.

### `dg-resume`

A control can resume an interaction by pointing at it with a selector:

```html
<button dg-resume="#system-live">Retry</button>
```

The runtime dispatches `dg:resume`, clears failure state, restarts polling, and
performs an immediate refresh.

---

## Connection state contract

`dg-connection` points to a neutral connection-status container, while `dg-retry`
optionally points to a retry control.

```html
<section
    id="system-live"
    dg-get="/fragments/system"
    dg-trigger="load, every 2s"
    dg-connection="#connection-status"
    dg-retry="#retry"
    dg-failure-limit="3"
    dg-pause-on-failure>
</section>

<div
    id="connection-status"
    data-dg-label-connecting="Connecting"
    data-dg-label-live="Live"
    data-dg-label-reconnecting="Reconnecting"
    data-dg-label-offline="Offline">
    <span data-dg-connection-label></span>
    <span data-dg-connection-detail hidden></span>
</div>

<button id="retry" dg-resume="#system-live" hidden>Retry</button>
```

The runtime writes the current state to:

```html
data-dg-connection-state="connecting|live|stale|reconnecting|offline"
```

A server-rendered fragment may override connection presentation by returning an
element with:

```html
data-dg-connection-state
data-dg-connection-label
data-dg-connection-detail
```

The browser runtime copies that state into the configured status container. This
keeps connection semantics independent from application-specific CSS and markup.

Drogular UI recognizes the standard connection states when the status container also
uses `dg-status`, but that styling is optional.

---

## Fragment preservation and availability

### `data-dg-preserve-key`

Open/closed state for keyed native `<details>` elements is preserved across fragment
replacement:

```html
<details data-dg-preserve-key="gpio-4">
    ...
</details>
```

When no previous keyed state exists, the first keyed `<details>` element is opened.

### `dg-hide-on-unavailable`

When this attribute is present on an interaction root, the root is hidden if the
returned fragment contains `data-dg-unavailable`.

---

## Resource API

The embedded JavaScript is also available to framework-level code through:

```cpp
#include <drogular/interactions_resources.hpp>

drogular::interactions_resources::ScriptPath;
drogular::interactions_resources::script();
```

Normal applications should prefer `App::interactions()` and the public asset path.
The resource API is useful for tests and framework integrations.

---

## Current scope

Drogular Interactions intentionally remains small. It currently performs GET-based
HTML fragment replacement; it is not a general-purpose client framework.

Interaction roots are discovered when the script initializes. Newly inserted
`dg-get` roots are not automatically installed by a mutation observer.

See the [Server-driven fragments Cookbook](../../cookbook/server-driven-interactions.md)
and the [System Monitor PWA](../../../examples/system_monitor_pwa/README.md) for a
complete working example.