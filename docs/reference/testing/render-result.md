# `RenderResult`

**Namespace:** `drogular::test`  
**Header:** `<drogular/testing.hpp>`  
**Kind:** Data structure

## Purpose

`RenderResult` stores the output of [`renderPage()`](render-page.md): the rendered HTML and the `RenderContext` used during the render.

## Synopsis

```cpp
struct RenderResult {
    std::string html;
    RenderContext context;
};
```

## Members

### `html`

```cpp
std::string html;
```

Contains the HTML returned by the page render.

### `context`

```cpp
RenderContext context;
```

Contains the rendering context after `renderPage()` completes.

This is useful for asserting values written by page initialization or rendering logic:

```cpp
const auto result =
    drogular::test::renderPage<MyPage>();

EXPECT_TRUE(result.context.contains("title"));
```

The context is moved into the result before `renderPage()` returns.

## Example

```cpp
const auto result =
    drogular::test::renderPage<MyPage>();

EXPECT_TRUE(
    drogular::test::contains(
        result.html,
        "Users"
    )
);

EXPECT_TRUE(
    result.context.contains("pageTitle")
);
```

## Related Types

- [`renderPage()`](render-page.md)
- [`RenderContext`](../rendering/render-context.md)
