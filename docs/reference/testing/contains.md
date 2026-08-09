# `contains()`

**Namespace:** `drogular::test`  
**Header:** `<drogular/testing.hpp>`  
**Kind:** Function

## Purpose

`contains()` is a small assertion helper that checks whether one string contains another.

## Synopsis

```cpp
bool contains(
    const std::string& text,
    const std::string& value
);
```

## Parameters

### `text`

The string to search.

### `value`

The substring to find.

## Return Value

Returns `true` when `value` occurs anywhere in `text` and `false` otherwise.

The implementation uses `std::string::find()` and performs a case-sensitive byte-string search.

An empty `value` returns `true`, matching `std::string::find("")` behavior.

## Example

```cpp
const auto result =
    drogular::test::renderPage<MyPage>();

EXPECT_TRUE(
    drogular::test::contains(
        result.html,
        "Dashboard"
    )
);
```

## Related Functions

- [`renderPage()`](render-page.md)
- [`renderComponentTree()`](render-component-tree.md)
