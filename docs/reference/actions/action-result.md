# `ActionResult`

**Namespace:** `drogular`  
**Header:** `<drogular/action_result.hpp>`  
**Kind:** Class

## Purpose

`ActionResult` describes the response produced by an `ActionHandler`.

It supports empty responses, redirects, HTML, JSON, files, downloads, and response cookies.

---

## Construction

`ActionResult` values are created through static factory methods.

### `empty()`

```cpp
static ActionResult empty();
```

Creates an `ActionResultType::Empty` result.

### `redirect()`

```cpp
static ActionResult redirect(
    std::string location
);
```

Creates a redirect result.

When converted by `toHttpResponse()`, the current implementation produces HTTP `302 Found`.

### `html()`

```cpp
static ActionResult html(
    std::string html
);
```

Creates an HTML result.

The converter sends the body with Drogon's `text/html` content type.

### `json()`

```cpp
static ActionResult json(
    Json::Value json
);
```

Creates a JSON result.

### `file()`

```cpp
static ActionResult file(
    std::filesystem::path path
);
```

Creates an inline file response using Drogular's static-file response infrastructure.

### `download()`

```cpp
static ActionResult download(
    std::filesystem::path path,
    std::string downloadName
);
```

Creates a file result marked for download.

The converter adds:

```text
Content-Disposition: attachment; filename="<downloadName>"
```

The current implementation inserts `downloadName` directly into the header value. Callers should provide a safe filename that does not contain header-control characters.

---

## Public API

### `type()`

```cpp
ActionResultType type() const;
```

Returns the stored result variant.

### `location()`

```cpp
const std::string& location() const;
```

Returns the redirect location storage.

### `body()`

```cpp
const std::string& body() const;
```

Returns the HTML body storage.

### `json()`

```cpp
const Json::Value& json() const;
```

Returns the JSON storage.

### `fileInfo()`

```cpp
const FileResponseInfo& fileInfo() const;
```

Returns file response metadata.

The type-specific accessors do not validate `type()`. Reading an accessor that does not correspond to the active result variant returns that field's current stored value, normally its default value.

---

## Cookies

### `cookie()`

```cpp
ActionResult& cookie(
    std::string name,
    std::string value,
    std::string path = "/",
    bool httpOnly = true
);
```

Appends a response cookie and returns `*this` for chaining.

```cpp
return drogular::ActionResult::redirect(
    "/dashboard"
).cookie(
    "session_id",
    sessionId
);
```

The current cookie model exposes only:

- name;
- value;
- path;
- HttpOnly.

Secure, SameSite, expiry, and Max-Age attributes are not represented by `ActionResult::cookie()`.

### `cookies()`

```cpp
const std::vector<Cookie>& cookies() const;
```

Returns all cookies attached to the result.

---

## HTTP Conversion Behavior

[`toHttpResponse()`](action-response.md) applies cookies to redirect, HTML, JSON, and file results.

For `ActionResultType::Empty`, the current converter returns a plain Drogon response before applying cookies. Cookies attached to an empty result are therefore currently ignored.

---

## Example

```cpp
Json::Value json;
json["ok"] = true;

return drogular::ActionResult::json(
    std::move(json)
).cookie(
    "request_complete",
    "1",
    "/",
    false
);
```

---

## Related Types

- [`ActionResultType`](action-result-type.md)
- [`Cookie`](cookie.md)
- [`FileResponseInfo`](file-response-info.md)
- [`toHttpResponse()`](action-response.md)
- [`ActionHandler`](action-handler.md)
