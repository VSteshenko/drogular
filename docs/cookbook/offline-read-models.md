# Offline Read-only Applications

Use Offline Read Models when selected server-rendered lists or dashboards should remain browsable after connectivity 
is lost, but application writes must remain server-authoritative.

## 1. Enable the capability

```cpp
app.offlineReadModels();
```

This enables Drogular Interactions as well. Load the runtime in the layout:

```html
<script src="/__drogular/assets/interactions.js" defer></script>
```

## 2. Opt a read interaction into exact caching

Start with a normal GET interaction and add the two offline markers:

```html
<section
    dg-get="/projects/fragments/browser"
    dg-target="[data-projects-results]"
    dg-trigger="load, change delay:150ms"
    dg-offline-read
    dg-offline-runtime="framework">

    <input name="search">
    <div data-projects-results>...</div>
</section>
```

Only this explicitly marked GET root participates. The framework seeds its initial server-rendered target and stores 
later successful GET results.

If the same request fails later, Drogular restores only the representation with the same normalized request, 
locale/context, and session scope.

## 3. Mark the application shell

Wrap the part of the page that should be restorable during offline navigation:

```html
<div dg-offline-shell>
    <header>...</header>
    <main>...</main>
</div>
```

Mark navigation links that may use previously cached shells:

```html
<a href="/projects" dg-offline-navigation>Projects</a>
<a href="/departments" dg-offline-navigation>Departments</a>
<a href="/users" dg-offline-navigation>Users</a>
```

When the runtime is read-only, these links perform same-origin cache-only navigation. They do not pretend that 
an unvisited route is available: a missing shell produces the `unavailable` data state.

## 4. Make rendering context explicit

Locale is automatically read from `<html lang>`. Add other dimensions only when they actually change server-rendered HTML:

```html
<html lang="de" data-dg-context-tenant="acme">
```

Do not use dimensions as arbitrary metadata. Every dimension expands representation identity and therefore the number 
of exact cache entries.

## 5. Clear cached representations at identity boundaries

Logout is the common case:

```html
<form method="post" action="/logout" data-dg-offline-clear>
    <button type="submit">Logout</button>
</form>
```

The current browser adapter uses a session scope, but explicit clearing is still the appropriate application behavior 
when identity changes.

## 6. Support cached locale switching when useful

```html
<form method="post" action="/language" dg-offline-locale="de">
    <button type="submit">Deutsch</button>
</form>
```

Online, this remains an ordinary server POST. In read-only mode, Drogular prevents the POST and attempts to restore 
already cached German shell/read representations. On reconnect, it reconciles the pending locale choice with the server.

## 7. Present read-only state without changing domain semantics

The root `<html>` receives:

```text
data-dg-connection-state
data-dg-data-state
data-dg-mode
```

Application CSS or a small presentation helper can use those values to explain that cached data is being shown and editing 
is unavailable.

Keep this layer presentational. PortalDemo, for example, adds application-specific filter-history UI around the framework 
cache. That helper is useful reference code, but it is not required by Offline Read Models.

## 8. Do not build offline mutation semantics accidentally

While cached data is active, Drogular blocks non-GET form submissions. Do not work around that by manually queueing 
arbitrary POST bodies unless the application has deliberately designed idempotency, ordering, authorization revalidation, 
conflicts, and reconciliation.

For the standard Drogular model, offline means **read cached server representations; write when live**.

## Checklist

- call `app.offlineReadModels()`;
- load `interactions.js`;
- opt safe GET roots into `dg-offline-read` + `dg-offline-runtime="framework"`;
- mark one restorable `dg-offline-shell` when cached navigation is desired;
- mark only appropriate links with `dg-offline-navigation`;
- clear the store on logout/identity boundaries;
- make locale and other rendering dimensions explicit;
- keep mutations server-only.

See the [Offline Read Models Reference](../reference/offline/README.md) for the exact runtime contract 
and [Offline Read Models Architecture](../architecture/offline-read-models.md) for the design rationale.