# `RoutePattern`

**Namespace:** `drogular`  
**Header:** `<drogular/route_pattern.hpp>`  
**Kind:** Class

## Purpose

`RoutePattern` performs segment-based path matching and extracts named parameters from patterns such as `/projects/{id}`.

The `Router` uses it to populate route parameters in `RenderContext` and `ActionContext`.

---

## Synopsis

```cpp
class RoutePattern {
public:
    explicit RoutePattern(
        std::string pattern
    );

    bool match(
        const std::string& path,
        std::unordered_map<std::string, std::string>& parameters
    ) const;
};
```

---

## Construction

```cpp
drogular::RoutePattern pattern(
    "/projects/{id}"
);
```

The pattern is parsed once when the object is constructed.

A segment is treated as a parameter only when the complete segment:

- has at least three characters;
- begins with `{`;
- ends with `}`.

For example, `{id}` is a parameter while `{}` is treated as a static segment.

---

## `match()`

```cpp
bool match(
    const std::string& path,
    std::unordered_map<std::string, std::string>& parameters
) const;
```

Returns `true` when `path` has the same number of parsed segments and every static segment matches exactly.

Named parameter segments accept any corresponding segment value and store it under the parameter name.

```cpp
drogular::RoutePattern pattern(
    "/projects/{projectId}/tasks/{taskId}"
);

std::unordered_map<std::string, std::string> parameters;

if (pattern.match(
        "/projects/10/tasks/25",
        parameters
    )) {
    // parameters["projectId"] == "10"
    // parameters["taskId"] == "25"
}
```

---

## Matching Rules

### Segment count must match

```text
Pattern: /projects/{id}
Path:    /projects
Result:  false
```

Extra path segments also fail.

### Static matching is exact

```text
Pattern: /projects/{id}
Path:    /users/42
Result:  false
```

String comparison is case-sensitive.

### Empty path segments are ignored

Parsing skips empty segments produced by `/` separators.

As a consequence, leading, trailing, and repeated slashes do not create additional segments for `RoutePattern` matching itself.

### Parameters are not URL-decoded

`RoutePattern` stores the path segment exactly as supplied to `match()`.

It does not perform percent decoding.

### Output map behavior

The implementation first collects matches in a temporary map and only inserts them into `parameters` after the entire pattern succeeds.

Therefore a failed match does not add partial parameter values.

On success, values are inserted with `unordered_map::insert()`:

- the supplied map is not cleared;
- unrelated existing entries remain;
- an existing key in the destination map is not overwritten by insertion.

If the same parameter name occurs more than once inside a pattern, the last matched segment wins in the temporary match map before insertion.

---

## Errors

`RoutePattern` does not throw for malformed route syntax.

Segments that do not satisfy the simple `{name}` recognition rule are treated as static text.

---

## Thread Safety

After construction, `RoutePattern::match()` does not mutate the pattern object.

Separate calls are safe with respect to the `RoutePattern` instance itself, provided each call supplies its own output map or externally synchronizes shared output state.

---

## Example

```cpp
drogular::RoutePattern pattern(
    "/users/{id}/edit"
);

std::unordered_map<std::string, std::string> params;

const bool matched = pattern.match(
    "/users/42/edit",
    params
);

if (matched) {
    const auto id = params.at("id");
}
```

---

## Related Types

- [`Router`](router.md)
- [`RenderContext`](../rendering/render-context.md)
- [`ActionContext`](../actions/action-context.md)

## See Also

- [Routing — Getting Started](../../getting-started/routing.md)
