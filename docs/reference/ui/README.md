# Drogular UI

Drogular UI is a small optional stylesheet containing reusable presentation
primitives for server-rendered Drogular applications.

It is intentionally not a complete CSS framework. Applications keep ownership of
layout, branding, typography, and domain-specific presentation while Drogular UI
provides a shared visual base for common controls and semantic states.

Enable it explicitly:

```cpp
app.ui();
```

Then load the built-in stylesheet:

```html
<link rel="stylesheet" href="/__drogular/assets/ui.css">
```

`App::ui()` is idempotent. It does not enable Drogular Interactions and does not
inject a `<link>` tag automatically.

---

## Primitives

### `dg-card`

A reusable bordered surface for grouping content.

```html
<section class="dg-card dashboard-panel">
    ...
</section>
```

### `dg-button`

A shared base for links or buttons used as application controls.

```html
<button class="dg-button">Retry</button>
```

### `dg-toolbar`

A horizontal flex container for compact groups of controls.

```html
<div class="dg-toolbar">
    ...
</div>
```

### `dg-status`

A pill-shaped status surface. Semantic variants are available through:

```text
dg-status-neutral
dg-status-info
dg-status-success
dg-status-warning
dg-status-danger
```

When used with Drogular Interactions, `dg-status` also recognizes standard
`data-dg-connection-state` values:

- `idle` → neutral
- `connecting` → info
- `live` → success
- `stale` / `reconnecting` → warning
- `offline` → danger

This is the only deliberate integration point between the two optional modules;
Drogular Interactions itself does not require Drogular UI.

### `dg-badge`

A compact semantic label. Variants:

```text
dg-badge-neutral
dg-badge-info
dg-badge-success
dg-badge-warning
dg-badge-danger
```

```html
<span class="dg-badge dg-badge-success">Available</span>
```

### `dg-segmented`

A shared container for compact mutually related controls. Child controls use
`dg-segmented-item`; the selected item uses `is-active`.

```html
<div class="dg-segmented">
    <button class="dg-segmented-item is-active">All</button>
    <button class="dg-segmented-item">Used</button>
</div>
```

---

## Composition model

Drogular UI classes are designed to be composed with application-owned classes:

```html
<section class="dg-card hardware-panel board-identity">
    ...
</section>
```

The framework primitive provides the common surface while the application class
owns domain layout and any intentional visual specialization.

Semantic variants should describe meaning rather than a specific color. For example,
server-side C++ can choose `success`, `warning`, or `danger`, while Drogular UI owns
the shared treatment.

---

## Resource API

The embedded stylesheet is also available through:

```cpp
#include <drogular/ui_resources.hpp>

drogular::ui_resources::StylesheetPath;
drogular::ui_resources::stylesheet();
```

Normal applications should prefer `App::ui()` and the built-in asset path. The
resource API is primarily useful in tests and framework integrations.

See [Drogular Interactions](../interactions/README.md) for optional declarative
fragment behavior and [System Monitor PWA](../../../examples/system_monitor_pwa/README.md)
for a complete application that combines both modules.