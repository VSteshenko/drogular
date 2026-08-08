# `Url`

**Namespace:** `drogular`  
**Header:** `<drogular/url.hpp>`  
**Kind:** Utility class

## Purpose

`Url` provides helpers for safely encoding values placed into URLs.

The current public API contains a single percent-encoding function intended for query-string values.

---

## Public API

### `encode()`

```cpp
static std::string encode(
    const std::string& value
);
```

Percent-encodes bytes that are not URL unreserved characters.

The following characters remain unchanged:

```text
A-Z  a-z  0-9  -  _  .  ~
```

All other bytes are encoded as `%HH` with uppercase hexadecimal digits.

```cpp
const auto value =
    drogular::Url::encode("hello world");

// "hello%20world"
```

Additional examples:

```text
abcXYZ-_.~123  -> abcXYZ-_.~123
a&b=c           -> a%26b%3Dc
```

For UTF-8 input, encoding operates on the individual bytes of the string. For example, UTF-8 `ä` becomes `%C3%A4`.

---

## Behavior

`encode()`:

- performs percent encoding only;
- does not build a complete query string;
- does not decode percent-encoded values;
- does not convert spaces to `+`;
- does not normalize or validate a complete URL.

Use [`QueryStringBuilder`](../pagination-and-request-parameters/query-string-builder.md) for higher-level query-string construction.

---

## Example

PortalDemo uses `Url::encode()` when preserving a return URL inside another query string:

```cpp
const auto redirect =
    "/users/login?returnUrl=" +
    drogular::Url::encode(returnUrl);
```

This prevents characters inside `returnUrl` from being interpreted as separators belonging to the outer query string.

---

## Related Types

- [`Router`](router.md)
- [`QueryStringBuilder`](../pagination-and-request-parameters/query-string-builder.md)

## See Also

- [Routing — Getting Started](../../getting-started/routing.md)
