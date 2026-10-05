# Offline Read Models

Drogular Offline Read Models extends the optional Interactions runtime with exact, server-rendered read representations
that can be restored when the network is unavailable. It is deliberately a **read-only** model: Drogular preserves 
server-rendered HTML and blocks mutation submissions while cached data is active instead of attempting client-side domain 
writes or conflict resolution.

Enable it explicitly:

```cpp
app.offlineReadModels();
```

This also enables `app.interactions()`. The application must still load the Interactions resource:

```html
<script src="/__drogular/assets/interactions.js" defer></script>
```

## Public C++ model

Header:

```cpp
#include <drogular/offline.hpp>
```

### `OfflineReadPolicy`

```cpp
enum class OfflineReadPolicy {
    Disabled,
    Exact
};
```

`Exact` describes the framework's current cache policy: only a representation with the same logical identity may satisfy 
an offline read. Drogular does not choose a nearest, stale-by-query, or semantically similar representation.

### Representation identity

```cpp
struct RepresentationIdentity {
    RepresentationKind kind;
    std::string requestKey;
    RepresentationContext context;
    RepresentationScope scope;
};
```

An identity has four independent dimensions:

- `kind` — `Fragment` or `Shell`;
- `requestKey` — normalized logical request identity;
- `context` — locale plus application-defined rendering dimensions;
- `scope` — the lifetime/security boundary of the cached representation.

`RepresentationContext::locale` is intentionally first-class because the same request may render different HTML in
different languages. `dimensions` allows other server-rendering inputs to participate in identity.

`RepresentationScopeKind` defines `Session`, `AuthenticatedPrincipal`, `Application`, and `Custom`. The current built-in 
browser runtime uses a generated **session scope**. The wider enum is the public identity model; it does not imply that 
the current browser adapter automatically derives authenticated-principal, application, or custom scope keys.

Applications must not depend on the IndexedDB serialization format of `RepresentationIdentity`.

### Runtime state

```cpp
struct OfflineState {
    ConnectionState connection;
    DataState data;
    InteractionCapability capability;
};
```

The three axes are independent:

- connection: `Live`, `Offline`, `Reconnecting`;
- data: `Live`, `Cached`, `Unavailable`;
- capability: `ReadWrite`, `ReadOnly`.

This avoids treating connectivity, data provenance, and allowed interaction as one ambiguous `offline` flag.

## Opting a GET interaction into caching

Offline fallback is explicit per read root:

```html
<section
    dg-get="/projects/fragments/browser"
    dg-target="[data-projects-results]"
    dg-offline-read
    dg-offline-runtime="framework">
    <div data-projects-results>...</div>
</section>
```

The built-in runtime requires all of the following:

- Offline Read Models are enabled;
- the root uses `dg-get`;
- `dg-offline-read` is present;
- `dg-offline-runtime="framework"` is present.

POST interactions are never restored from the representation store.

The initial server-rendered target is seeded into the store. A successful network GET replaces the exact stored 
representation. If the network request later fails, the runtime looks up that exact identity and restores it when available.

## Exact request identity

Fragment request keys use the URL path plus normalized query parameters. Query parameters are sorted before the key 
is created, so equivalent parameter order does not create a different representation.

The representation context is derived from:

- `<html lang="...">` for locale;
- `<html data-dg-context-NAME="...">` for additional dimensions.

For example:

```html
<html lang="de" data-dg-context-tenant="acme">
```

produces a context containing locale `de` and dimension `tenant=acme`.

## Storage and scope

The built-in adapter stores representations in IndexedDB database `drogular-offline-representations`. Its storage format 
is an implementation detail.

The current runtime creates a random session scope key in `sessionStorage`. This prevents one browser session's exact 
representations from being selected under another session scope. Clearing the offline store also removes that scope key.

A representation cache is not an authorization mechanism. The server remains responsible for authorization when producing 
live representations, and applications should clear cached representations at identity boundaries such as logout.

A form marked with `data-dg-offline-clear` clears the framework representation store when it is submitted:

```html
<form method="post" action="/logout" data-dg-offline-clear>
    ...
</form>
```

## Cached application shells

Mark the replaceable application shell:

```html
<div dg-offline-shell>
    ...
</div>
```

When Offline Read Models are enabled, the current shell is stored using a `Shell` representation identity. Shell identity 
uses the pathname rather than fragment query parameters.

Links explicitly marked for offline navigation can restore a cached same-origin shell while the runtime is read-only:

```html
<a href="/projects" dg-offline-navigation>Projects</a>
```

The runtime also handles browser `popstate` while in read-only mode. If the requested shell is unavailable, it publishes 
`data=unavailable` and dispatches `dg:offline-navigation-unavailable`.

## Locale while offline

A locale form may opt into cache-only locale switching:

```html
<form method="post" action="/language" dg-offline-locale="de">
    ...
</form>
```

While read-only, the runtime does not POST the locale change. It attempts to restore the current shell and offline read 
roots using the requested locale as part of representation identity. When connectivity returns, the pending locale request 
is reconciled with the server before normal live navigation resumes.

This preserves the rule that locale is server-owned while still allowing already cached localized representations to be 
browsed offline.

## Read-only enforcement

When cached data becomes active, the runtime publishes `capability=read-only`. Non-GET form submissions are then 
intercepted and blocked. Offline Read Models do not queue mutations for later replay.

This is a deliberate consistency boundary: cached server representations remain readable, but application writes require a live server.

## Browser state contract

The runtime publishes its state on `<html>`:

```text
data-dg-connection-state="live|offline|reconnecting"
data-dg-data-state="live|cached|unavailable"
data-dg-mode="read-write|read-only"
```

State changes also dispatch:

```text
dg:offline-state
```

with the three state values in `event.detail`.

When a representation is stored, the runtime dispatches `dg:offline-representation-stored` with its identity.

For diagnostics and application-level presentation, the enabled runtime exposes:

```js
globalThis.drogularOfflineReadModels.representations()
globalThis.drogularOfflineReadModels.clear()
globalThis.drogularOfflineReadModels.state()
```

Treat this object as a small runtime integration surface, not as a replacement for the C++ identity model.

## Relationship to PWA offline support

Offline Read Models and the PWA helpers solve different problems:

- `App::offlinePage()` registers a fallback Page that a service worker may choose to serve;
- Offline Read Models preserve exact server-rendered representations inside the Interactions runtime.

An application may use either or both. Offline Read Models do not generate service-worker fetch rules.

## Current boundaries

The framework currently provides exact GET representation caching, session-scoped browser storage, cached shell navigation, 
locale/context-aware identity, explicit read-only state, and mutation blocking.

It does **not** provide offline writes, mutation queues, conflict resolution, approximate representation matching, 
or automatic authenticated-principal/custom scope derivation.

See also [Drogular Interactions](../interactions/README.md), 
[Offline Read Models Architecture](../../architecture/offline-read-models.md), 
and the [Offline Read-only Applications Cookbook](../../cookbook/offline-read-models.md).