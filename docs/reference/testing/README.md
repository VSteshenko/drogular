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

## Lifecycle Parity

`renderPage()` and `renderComponentTree()` delegate to the same production lifecycle runner used by page routing.

A page or component therefore follows the same sequence in tests and at runtime:

```text
onInit() -> render() -> onDestroy()
```

`onInit()` runs exactly once. `onDestroy()` also runs when rendering throws after initialization, and the original exception is propagated.

The testing helpers still bypass HTTP route registration and response construction, but they no longer maintain a separate lifecycle implementation.

## Related Documentation

- [Pages](../pages/README.md)
- [Components](../components/README.md)
- [Rendering](../rendering/README.md)
