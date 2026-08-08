# `Cookie`

**Namespace:** `drogular`  
**Header:** `<drogular/action_result.hpp>`  
**Kind:** Struct

## Purpose

`Cookie` stores the cookie metadata attached to an `ActionResult`.

---

## Synopsis

```cpp
struct Cookie {
    std::string name;
    std::string value;
    std::string path = "/";
    bool httpOnly = true;
};
```

---

## Members

### `name`

Cookie name.

### `value`

Cookie value.

### `path`

Cookie path. Defaults to `/`.

### `httpOnly`

Controls the HttpOnly attribute. Defaults to `true`.

---

## Behavior

Cookies are normally appended through `ActionResult::cookie()` rather than constructed directly.

When an action result is converted to a Drogon response, the converter currently maps `name`, `value`, `path`, and `httpOnly` only.

The struct does not currently represent Secure, SameSite, expiry, or Max-Age attributes.

---

## Example

```cpp
return drogular::ActionResult::redirect(
    "/dashboard"
).cookie(
    "session_id",
    sessionId
);
```

---

## Related Types

- [`ActionResult`](action-result.md)
- [`toHttpResponse()`](action-response.md)
