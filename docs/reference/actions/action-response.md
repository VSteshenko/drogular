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

Cookies are translated to `drogon::Cookie` values for every result type, including `Empty`.

The converter maps path, HttpOnly, Secure, SameSite, and Max-Age when those options are set.

---

## Error Conversion

The action pipeline also uses `toHttpErrorResponse()` when an action throws:

```cpp
drogon::HttpResponsePtr toHttpErrorResponse(
    const std::exception& error
);

drogon::HttpResponsePtr toHttpErrorResponse();
```

[`ActionValidationError`](../forms-and-validation/action-validation-error.md) becomes `400 Bad Request` with the validation message in a plain-text body. Other `std::exception` values and unknown exceptions become `500 Internal Server Error` with the fixed body `Internal Server Error`; their internal exception messages are not exposed to the client.

See [Action Error Handling](error-handling.md) for the framework contract.

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
