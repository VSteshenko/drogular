# `toHttpResponse()`

**Namespace:** `drogular`  
**Header:** `<drogular/action_response.hpp>`  
**Kind:** Free function

## Purpose

`toHttpResponse()` converts an `ActionResult` into a Drogon `HttpResponse`.

The action router calls this function automatically after `ActionHandler::handle()` returns.

---

## Signature

```cpp
drogon::HttpResponsePtr toHttpResponse(
    const ActionResult& result
);
```

---

## Conversion Rules

| `ActionResultType` | HTTP behavior |
| --- | --- |
| `Empty` | Creates `drogon::HttpResponse::newHttpResponse()`. |
| `Redirect` | Creates a Drogon redirection response. Current status is `302 Found`. |
| `Html` | Creates a response with `text/html` content type and the stored body. |
| `Json` | Uses `drogon::HttpResponse::newHttpJsonResponse()`. |
| `File` | Uses `StaticFileResponse::create()` for the stored path. |

For forced downloads, the function also adds:

```text
Content-Disposition: attachment; filename="<downloadName>"
```

---

## Cookies

Cookies are translated to `drogon::Cookie` values for:

- redirects;
- HTML;
- JSON;
- files/downloads.

The converter sets the cookie path and enables HttpOnly when requested.

The current `Empty` branch returns before cookie application, so cookies attached to `ActionResult::empty()` are not emitted.

---

## Example

Applications normally return an `ActionResult` and let the router perform conversion:

```cpp
return drogular::ActionResult::json(json);
```

Direct conversion is useful in framework-level code or tests:

```cpp
const auto response =
    drogular::toHttpResponse(
        drogular::ActionResult::redirect("/")
    );
```

---

## Related Types

- [`ActionResult`](action-result.md)
- [`ActionResultType`](action-result-type.md)
- [`Cookie`](cookie.md)
- [`FileResponseInfo`](file-response-info.md)
