# `Cookie`

**Namespace:** `drogular`  
**Header:** `<drogular/action_result.hpp>`  
**Kind:** Struct

## Purpose

`Cookie` stores the response-cookie metadata attached to an `ActionResult`.

---

## Synopsis

```cpp
enum class CookieSameSite {
    Unspecified,
    Lax,
    Strict,
    None
};

struct CookieOptions {
    std::string path = "/";
    bool httpOnly = true;
    bool secure = false;
    CookieSameSite sameSite = CookieSameSite::Unspecified;
    std::optional<int> maxAge;
};

struct Cookie {
    std::string name;
    std::string value;
    std::string path = "/";
    bool httpOnly = true;
    bool secure = false;
    CookieSameSite sameSite = CookieSameSite::Unspecified;
    std::optional<int> maxAge;
};
```

---

## Members

`name` and `value` contain the cookie pair. `path` defaults to `/`. `httpOnly` defaults to `true`, `secure` defaults to `false`, `sameSite` defaults to `Unspecified`, and `maxAge` is omitted by default.

`CookieSameSite` maps to the corresponding Drogon SameSite value when a result is converted to an HTTP response. `Unspecified` leaves the attribute unset.

---

## Recommended Construction

Cookies are normally appended through `ActionResult::cookie()` rather than constructed directly. The compatibility overload covers the common path/HttpOnly case:

```cpp
return drogular::ActionResult::redirect("/dashboard")
    .cookie("session_id", sessionId);
```

Use `CookieOptions` when security attributes are required:

```cpp
drogular::CookieOptions options;
options.httpOnly = true;
options.secure = true;
options.sameSite = drogular::CookieSameSite::Lax;
options.maxAge = 3600;

return drogular::ActionResult::redirect("/dashboard")
    .cookie("session_id", sessionId, options);
```

`Max-Age` is expressed in seconds.

---

## Related Types

- [`ActionResult`](action-result.md)
- [`toHttpResponse()`](action-response.md)
