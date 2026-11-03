# Safe Return Navigation

**Problem:** A user opens an item from a filtered list, edits it, and should return to the same list state without 
allowing an arbitrary redirect destination.

**Recommended solution:** Validate incoming `returnUrl` values with `NavigationContext::resolve()` and propagate only 
validated destinations with `NavigationContext::propagate()`.

---

## 1. Resolve the return destination

```cpp
#include <drogular/navigation_context.hpp>

const auto safeReturnUrl = drogular::NavigationContext::resolve(
    requestedReturnUrl,
    "/projects",
    "/projects"
);
```

The third argument restricts the destination to `/projects` and its query string. A different route or a lookalike 
prefix falls back to `/projects`.

For nested list routes, use `allowDescendants = true` only when those descendants are intended return destinations.

---

## 2. Preserve list state in links

```cpp
const auto editUrl = drogular::NavigationContext::propagate(
    "/projects/42/edit",
    safeReturnUrl
);
```

For a list URL such as `/projects?search=active&page=2`, the generated edit URL contains an encoded `returnUrl` 
query parameter. `propagate()` does not validate its second argument; always resolve untrusted input first.

---

## 3. Validate again after a POST

The return destination may be submitted as a hidden form field. Do not trust it merely because the server previously 
rendered the form.

```cpp
const auto destination = drogular::NavigationContext::resolve(
    submittedReturnUrl,
    "/projects",
    "/projects"
);

return drogular::ActionResult::redirect(destination);
```

This navigation check does not replace authorization of the edited project or CSRF protection for state-changing requests.

---

## 4. Progressive enhancement

The same validated destination can be used by a regular POST redirect and an interaction-aware workflow. `dg-post` and
browser history controls handle presentation/navigation; `NavigationContext` handles validation of the server-side 
return destination.

See [Server-driven Interactions](server-driven-interactions.md) for the fragment-response flow and [NavigationContext Reference](../reference/routing/navigation-context.md) for exact API semantics.
