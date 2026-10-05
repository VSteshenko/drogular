# Offline Read Models Architecture

Offline Read Models preserve the server-rendered UI model when connectivity disappears without introducing a second 
client-side domain model.

The central rule is:

> Offline data is an exact cached server representation, not a mutable local copy of application state.

## Pipeline

```text
Live GET interaction
        │
        ▼
Server-rendered HTML
        │
        ├──► browser target
        │
        └──► RepresentationIdentity
                  │
                  ▼
               IndexedDB

Network failure
        │
        ▼
Same exact RepresentationIdentity
        │
        ▼
Cached HTML ──► browser target
        │
        ▼
read-only capability
```

This keeps templates, localization, authorization decisions, formatting, and presentation logic server-owned. The browser 
stores the result rather than reimplementing how the result is produced.

## Identity is part of correctness

A representation is identified by kind, normalized request key, rendering context, and scope. A cache hit is valid only 
when the identity matches exactly.

Locale belongs in identity because localized HTML is a different representation. Additional `data-dg-context-*` dimensions 
let an application make other rendering inputs explicit rather than hiding them in cache behavior.

Scope is a security/lifetime boundary. The public model supports session, authenticated-principal, application, and 
custom scopes. The current browser runtime intentionally implements session scope only. Supporting another scope requires 
a trustworthy way to derive and rotate its key; the enum alone does not provide that policy.

## Three independent runtime states

Offline-capable UI should not collapse all degraded behavior into one boolean. Drogular tracks:

```text
Connection: live | offline | reconnecting
Data:       live | cached | unavailable
Capability: read-write | read-only
```

For example, the browser may have regained connectivity (`reconnecting`) while still showing cached data (`cached`) and 
therefore remain `read-only` until live context is reconciled.

## Why writes are blocked

The runtime blocks non-GET forms while cached representations are active. It does not enqueue writes.

Queueing mutations would introduce substantially different semantics: durable commands, replay ordering, idempotency, 
authorization changes, conflicts, and user-visible reconciliation. Those concerns do not belong implicitly inside a representation cache.

Drogular therefore makes the safe boundary explicit: offline representations are readable; mutations require the server.

## Shells and fragments share the store

Two representation kinds use the same identity/storage model:

- `Fragment` — exact GET interaction output;
- `Shell` — an application shell identified by route pathname.

This allows cached navigation between previously visited application sections without turning navigation into a client 
router. The browser restores server-rendered shells and then restores exact cached read roots within them.

## Locale reconciliation

Offline locale switching is cache-only. The browser may select an already cached representation for another locale, but 
it cannot mutate the server session while disconnected.

The requested locale change is retained as pending context. On reconnect, the runtime reconciles that choice with 
the server before returning to normal live behavior. This prevents the browser's visible locale and server session 
locale from silently diverging.

## Application versus framework responsibilities

Framework responsibilities:

- representation identity model;
- exact matching;
- IndexedDB adapter;
- runtime state publication;
- GET fallback;
- shell restore/navigation;
- read-only mutation boundary;
- locale/context-aware lookup.

Application responsibilities:

- choose which GET roots are safe/useful to cache;
- render correct authorized representations;
- mark the shell and offline-capable navigation;
- clear cache at appropriate identity boundaries;
- provide user-facing offline/read-only presentation;
- decide any application-specific context dimensions.

PortalDemo intentionally keeps filter-history presentation in application code. That presentation is an example of 
composing the framework state, not part of the Offline Read Models API.

## Relationship to PWA

Service-worker fallback and Offline Read Models are orthogonal. A service worker controls network/resource behavior. 
Offline Read Models control exact server-rendered interaction representations and read-only application behavior. 
Combining them can provide an installable shell plus meaningful cached application data, but neither abstraction silently 
owns the other.