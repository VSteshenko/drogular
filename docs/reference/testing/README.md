# Testing

Drogular provides lightweight rendering helpers for unit-style tests of pages and component trees without starting a Drogon server.

The testing API lives in the `drogular::test` namespace and is available through:

```cpp
#include <drogular/testing.hpp>
```

## Types and Functions

- [`RenderResult`](render-result.md) — stores rendered HTML together with the `RenderContext` used by `renderPage()`.
- [`renderPage()`](render-page.md) — default-constructs a page and renders it with optional application services and request data.
- [`renderComponentTree()`](render-component-tree.md) — runs a component lifecycle and renders default and named child slots.
- [`contains()`](contains.md) — checks whether rendered text contains a substring.

## Typical Flow

```text
Test
 │
 ├── ApplicationServices (optional)
 ├── HttpRequest         (optional)
 │
 ▼
renderPage<PageType>()
 │
 ▼
RenderResult
 ├── html
 └── RenderContext
```

For lower-level component tests:

```text
Component + RenderContext
          │
          ▼
renderComponentTree()
          │
          ▼
         HTML
```

These helpers exercise Drogular rendering code directly. They do **not** register routes, start Drogon, perform an HTTP request through `Router`, or execute actions.

## Current Lifecycle Difference

The current implementation of `renderPage()` calls `PageType::onInit()` once directly and then passes the page to `renderComponentTree()`, which calls `onInit()` again.

As a result, `renderPage()` currently invokes page initialization **twice** and `onDestroy()` once. This differs from production page routing, where `Router` calls `onInit()` once and does not currently call `onDestroy()`.

Tests whose `onInit()` has side effects should account for this current behavior.

## Related Documentation

- [Pages](../pages/README.md)
- [Components](../components/README.md)
- [Rendering](../rendering/README.md)
