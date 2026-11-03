# `NavigationContext`

**Namespace:** `drogular`
**Header:** `<drogular/navigation_context.hpp>`
**Kind:** Utility class

## Purpose

`NavigationContext` validates and propagates application-local return destinations. Treat a `returnUrl` from a query 
parameter or hidden form field as untrusted input, even when the application originally generated it.

---

## Public API

### `isLocalTarget()`

```cpp
static bool isLocalTarget(std::string_view target);
```

Accepts an absolute application-local path beginning with a single `/`, optionally followed by a query string. Rejects 
empty and relative targets, protocol-relative URLs (`//...`), backslashes, fragments (`#`), whitespace/control characters, 
and percent-encoding in the **path** portion. Query-string percent-encoding is allowed.

This is a syntactic local-target check, not an authorization check or a guarantee that the route exists.

### `resolve()`

```cpp
static std::string resolve(
    std::string_view candidate,
    std::string_view fallback,
    std::string_view basePath = {},
    bool allowDescendants = false
);
```

Returns `candidate` if it is a valid local target and matches the optional `basePath` restriction. Otherwise returns 
a valid local `fallback`, or `/` if the fallback itself is invalid.

When `basePath` is provided, the candidate must match the base path exactly or continue with a query string. 
With `allowDescendants = true`, it may also continue with `/`. Lookalike prefixes such as `/projects-other` do not match 
`/projects`. The base path must itself be a valid local path without a query string.

```cpp
#include <drogular/navigation_context.hpp>

const auto returnUrl = drogular::NavigationContext::resolve(
    candidateReturnUrl,
    "/projects",
    "/projects"
);
// "/projects?search=active&page=2" is accepted.
// "/projects-other" and "/departments" fall back to "/projects".
```

### `propagate()`

```cpp
static std::string propagate(
    std::string_view destination,
    std::string_view returnUrl
);
```

Appends a percent-encoded `returnUrl` query parameter to a valid local `destination`, using `?` or `&` as appropriate. 
If `destination` is not a valid local target, returns `/`.

**Important:** `propagate()` encodes the supplied return URL but does **not** validate it. Resolve an untrusted candidate first.

```cpp
const auto safeReturnUrl = drogular::NavigationContext::resolve(
    candidateReturnUrl,
    "/projects",
    "/projects"
);

const auto editUrl = drogular::NavigationContext::propagate(
    "/projects/42/edit",
    safeReturnUrl
);
// /projects/42/edit?returnUrl=%2Fprojects%3Fsearch%3Dactive
```

---

## Recommended Return Flow

1. Validate the incoming `returnUrl` with `resolve()` and an appropriate collection base path.
2. Use `propagate()` to carry the validated value through detail/edit links.
3. On form submission, validate the submitted `returnUrl` again before redirecting. Hidden fields are user-controlled.
4. Keep authentication and authorization checks independent of navigation validation.

For example, a Projects list can preserve its search and pagination query when opening a project editor, then safely 
return to the list after a POST.

---

## Limitations

- The helper does not decode, canonicalize, or authorize URLs.
- The helper does not validate route existence or restrict a query string's parameter names.
- A caller that needs to restrict a return target to a specific section should pass `basePath` to `resolve()`; `isLocalTarget()` alone is insufficient.
- `propagate()` is a query-parameter helper, not a redirect or a general URL builder.

---

## Related Types

- [`Url`](url.md) — low-level percent encoding.
- [`Router`](router.md) — request routing.
- [`ActionResult`](../actions/action-result.md) — action responses and redirects.

## See Also

- [Safe Return Navigation — Cookbook](../../cookbook/safe-return-navigation.md)
- [Routing — Getting Started](../../getting-started/routing.md)
