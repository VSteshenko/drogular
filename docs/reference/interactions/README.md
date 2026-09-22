# Drogular Interactions

Drogular Interactions is an optional browser runtime for progressively enhanced, server-rendered HTML fragments. It keeps orchestration declarative while rendering, validation, localization, filtering, and domain presentation remain server-owned.

Enable it explicitly:

```cpp
app.interactions();
```

Then load the built-in script:

```html
<script src="/__drogular/assets/interactions.js" defer></script>
```

`App::interactions()` is idempotent. It does not enable Drogular UI and does not inject the script tag automatically.

## Interaction roots

Elements with `dg-get` or `dg-post` are interaction roots.

### `dg-get`

Named enabled controls are serialized into the GET query string. Unchecked checkboxes/radio buttons are ignored.

```html
<section dg-get="/fragments/projects"
         dg-target="[data-projects]"
         dg-trigger="load, input delay:250ms">
    <input name="search">
    <div data-projects></div>
</section>
```

### `dg-post`

Use `dg-post` for progressively enhanced form commands:

```html
<form dg-post="/projects/create"
      dg-target="[data-project-form]"
      dg-on-success-refresh="[data-projects-browser]">
    ...
    <div data-project-form></div>
</form>
```

POST requests use `application/x-www-form-urlencoded;charset=UTF-8`. Named enabled controls are serialized into the body and the active submitter's name/value is included when present.

Both GET and POST interactions send:

```text
Accept: text/html
X-Drogular-Interaction: true
```

Actions can detect this with `ActionContext::isInteraction()`.

A POST interaction may consume returned HTML from a non-2xx response. This allows an Action to return a validation fragment with `400 Bad Request`; the fragment is inserted into `dg-target` and the interaction state becomes `error`.

## Targeting

### `dg-target`

Selects the element whose `innerHTML` is replaced.

- for `dg-get`, an omitted target defaults to the interaction root;
- for `dg-post`, an omitted target means no fragment replacement;
- `this` explicitly targets the root;
- other selectors are resolved inside the root first, then against the document.

After replacement, nested `[dg-get]` and `[dg-post]` roots inside the new fragment are installed automatically. The runtime does not use a general-purpose `MutationObserver`; interaction roots inserted by unrelated application code are not automatically discovered.

## Triggers

### `dg-trigger`

Supported forms are `load`, `every Ns`, `input`, `input delay:Nms`, `change`, and `change delay:Nms`. Multiple triggers are comma-separated.

The default is `load` for `dg-get` and `submit` for `dg-post`. Forms intercept normal submit and pass the active submitter into request serialization.

## Request state

The root receives `dg-loading`, `dg-ready`, `dg-empty`, or `dg-error`. The same state is written to `data-dg-state`; `aria-busy` is updated while loading. A successful fragment containing `data-dg-empty` produces the empty state.

## URL history

### `dg-history`

Set `replace` or `push` to synchronize the browser URL after a successful interaction:

```html
<form dg-get="/fragments/projects"
      dg-history="replace"
      dg-history-url="/projects">
```

The URL is built from current named controls. Empty values and controls equal to their `dg-reset-value` are omitted.

### `dg-history-url`

Overrides the path used for history synchronization. For forms, `action` is the next fallback; otherwise the current pathname is used.

### `dg-current-url`

A control with `dg-current-url` is populated with the current path/query/hash immediately before a normal form submission. This supports redirect-back flows that also work without JavaScript.

## Success behavior

### `dg-on-success-navigate`

Navigates after a successful interaction:

```html
<form dg-post="/projects/1/delete"
      dg-on-success-navigate="/projects">
```

### `dg-on-success-refresh`

Refreshes other matching interaction roots after success:

```html
<form dg-post="/projects/create"
      dg-on-success-refresh="[data-projects-browser]">
```

The submitting root itself is not refreshed by this selector.

## Form reset helpers

### `dg-reset`

Inside an interaction form, a control with `dg-reset` resets the form, restores explicit reset values, and refreshes the interaction.

### `dg-reset-value`

Defines the value restored by `dg-reset`. The same value is treated as a history default and omitted from synchronized query parameters.

## Polling and pause/resume

`dg-poll-group` coordinates roots as a group. `dg-failure-limit` sets consecutive failures before offline state when `dg-connection` is present. `dg-pause-on-failure` stops polling at the limit.

A retry control can resume a root:

```html
<button dg-resume="#system-live">Retry</button>
```

The runtime clears failure state, restarts polling, and refreshes immediately.

## Connection state

`dg-connection` points to a status container and `dg-retry` may point to a retry control. The runtime writes:

```text
data-dg-connection-state="connecting|live|stale|reconnecting|offline"
```

A server fragment may provide `data-dg-connection-state`, `data-dg-connection-label`, and `data-dg-connection-detail`. Drogular UI can style this contract through `dg-status`, but UI remains optional.

## Fragment preservation and availability

`data-dg-preserve-key` preserves open/closed state for keyed native `<details>` across replacement. `dg-hide-on-unavailable` hides a root when the returned fragment contains `data-dg-unavailable`.

## Resource API

```cpp
#include <drogular/interactions_resources.hpp>

drogular::interactions_resources::ScriptPath;
drogular::interactions_resources::script();
```

Normal applications should prefer `App::interactions()` and the public asset path.

## Current scope

Drogular Interactions deliberately remains a small HTML-over-the-wire runtime, not a general-purpose client framework. It supports GET fragment queries, POST form commands, target replacement, request state, URL/history synchronization, success navigation/refresh, polling/retry, and a small set of form helpers.

See the [Server-driven interactions Cookbook](../../cookbook/server-driven-interactions.md), [ActionRenderer](../actions/action-renderer.md), [Portal Demo](../../../examples/portal_demo/), and [System Monitor PWA](../../../examples/system_monitor_pwa/README.md).
