# `ActionResultType`

**Namespace:** `drogular`  
**Header:** `<drogular/action_result.hpp>`  
**Kind:** Enum

## Purpose

`ActionResultType` identifies the response variant represented by an `ActionResult`.

---

## Values

```cpp
enum class ActionResultType {
    Empty,
    Redirect,
    Html,
    Json,
    File
};
```

| Value | Meaning |
| --- | --- |
| `Empty` | Plain empty/default HTTP response. |
| `Redirect` | HTTP redirect response. |
| `Html` | HTML body response. |
| `Json` | JSON response. |
| `File` | Inline file or forced download response. |

Both `ActionResult::file()` and `ActionResult::download()` use `File`; `FileResponseInfo::forceDownload` distinguishes their behavior.

---

## Example

```cpp
const auto result =
    drogular::ActionResult::redirect("/");

if (result.type() ==
    drogular::ActionResultType::Redirect) {
    // ...
}
```

---

## Related Types

- [`ActionResult`](action-result.md)
- [`FileResponseInfo`](file-response-info.md)
