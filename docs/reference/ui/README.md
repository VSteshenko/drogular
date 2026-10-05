# Drogular UI

Drogular UI is an optional presentation foundation for server-rendered Drogular applications. It provides shared design 
tokens, layout/form/table/navigation primitives, semantic state variants, and one small browser behavior for persistent 
collapsible panels.

It is intentionally not a complete CSS or client framework. Applications keep ownership of branding, domain presentation, 
and application-specific behavior.

Enable it explicitly:

```cpp
app.ui();
```

This publishes two resources:

```html
<link rel="stylesheet" href="/__drogular/assets/ui.css">
<script src="/__drogular/assets/ui.js" defer></script>
```

`App::ui()` is idempotent. It does not enable Drogular Interactions and does not inject either resource into a layout 
automatically. The script is needed only when using the persistent collapsible behavior described below; the stylesheet
can be used independently.

## Design tokens and themes

Drogular UI defines `--dg-*` custom properties for background/surfaces, text, borders, accent/focus colors, semantic colors,
shadows, and radii.

The stylesheet recognizes:

```text
data-dg-theme="light"
data-dg-theme="dark"
data-dg-theme="system"
```

`system` follows `prefers-color-scheme`. Applications may compose these tokens with their own CSS rather than duplicating 
framework colors and spacing decisions.

## Cards and composition

Core card primitives:

```text
dg-card
dg-card-header
dg-card-body
dg-card-footer
dg-card-title
dg-card-subtitle
dg-card-grid
dg-card-summary
dg-stack
dg-link-list
```

Example:

```html
<section class="dg-card project-summary">
    <header class="dg-card-header">
        <h2 class="dg-card-title">Project</h2>
        <p class="dg-card-subtitle">Current status</p>
    </header>
    <div class="dg-card-body">...</div>
</section>
```

Application-owned classes such as `project-summary` remain the correct place for domain-specific layout or branding.

## Persistent collapsible cards

Use native `<details>` with `dg-collapsible`:

```html
<details class="dg-card dg-collapsible" data-dg-collapse-key="projects.filters">
    <summary class="dg-card-summary">Filters</summary>
    <div class="dg-card-body">...</div>
</details>
```

When `/__drogular/assets/ui.js` is loaded, `data-dg-collapse-key` persists the `open`/`closed` state in `localStorage` 
under a Drogular-owned key prefix. Storage failures are ignored, so the native `<details>` behavior still works in 
restricted/private contexts.

Use stable, application-unique collapse keys. This UI persistence is independent from Interactions' `data-dg-preserve-key`, 
which preserves disclosure state across fragment replacement in memory.

## Forms

Form primitives:

```text
dg-form
dg-form-grid
dg-field
dg-label
dg-input
dg-select
dg-form-actions
dg-fieldset
dg-fieldset-legend
```

They style native controls while preserving ordinary HTML form semantics, making them suitable for progressively enhanced 
`dg-post` forms.

## Tables, details, empty states, and pagination

```text
dg-table-container
dg-table
dg-details
dg-details-item
dg-details-label
dg-details-value
dg-empty-state
dg-pagination
```

These are presentation primitives only. Pagination parameters and server-side data selection remain application/framework
request logic rather than UI behavior.

## Controls and semantic state

### `dg-button`

Shared base for button and link controls.

### `dg-toolbar`

Horizontal flex container for compact related controls.

### `dg-status`

Status surface with semantic variants:

```text
dg-status-neutral
dg-status-info
dg-status-success
dg-status-warning
dg-status-danger
```

It also recognizes Interactions connection state on the same element:

```text
data-dg-connection-state="idle|connecting|live|stale|reconnecting|offline"
```

### `dg-badge`

Compact label with `neutral`, `info`, `success`, `warning`, and `danger` variants.

### `dg-segmented`

Container for related controls. Children use `dg-segmented-item`; the selected item uses `is-active`.

## Application shell and navigation

Drogular UI now includes a responsive application-shell vocabulary:

```text
dg-shell
dg-sidebar
dg-brand
dg-brand-mark
dg-nav
dg-nav-group
dg-nav-item
dg-nav-submenu
dg-nav-subitem
dg-sidebar-footer
dg-sidebar-user
dg-topbar
dg-topbar-context
dg-topbar-actions
dg-main
dg-page
dg-page-header
dg-page-kicker
dg-page-title
dg-page-content
dg-footer
```

Navigation items recognize `is-active` and `aria-current="page"`. The shell collapses to a mobile-friendly block/navigation 
layout at the framework breakpoint.

These classes provide structural presentation, not routing. Links remain normal server routes and may independently opt 
into `dg-offline-navigation` or other Interactions behavior.

## Resource API

```cpp
#include <drogular/ui_resources.hpp>

drogular::ui_resources::StylesheetPath;
drogular::ui_resources::ScriptPath;
drogular::ui_resources::stylesheet();
drogular::ui_resources::script();
```

Normal applications should prefer `App::ui()` and the built-in asset paths. Direct resource access is primarily useful 
in tests and framework integrations.

## Composition with Interactions

Drogular UI and Drogular Interactions are independent:

```cpp
app.ui();            // presentation resources
app.interactions();  // HTML-over-the-wire behavior
```

They deliberately share a few semantic contracts, such as connection-state styling, but neither module requires the other.

PortalDemo demonstrates the broader shell/form/table/navigation primitives. System Monitor PWA demonstrates semantic status 
presentation and live Interactions behavior.

See [Drogular Interactions](../interactions/README.md) and [Offline Read Models](../offline/README.md).