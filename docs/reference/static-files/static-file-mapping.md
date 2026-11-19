# `StaticFileMapping`

**Namespace:** `drogular`  
**Header:** `<drogular/static_file_mapping.hpp>`  
**Kind:** Struct

## Purpose

`StaticFileMapping` associates a public URL route prefix with a filesystem directory.

`ApplicationOptions` stores these mappings and `App::run()` later registers each one with `Router::staticFiles()`.

---

## Synopsis

```cpp
struct StaticFileMapping {
    std::string routePrefix;
    std::filesystem::path directory;
    bool recursive = false;
};
```

---

## Members

### `routePrefix`

```cpp
std::string routePrefix;
```

The public URL prefix.

For example:

```text
/assets
```

### `directory`

```cpp
std::filesystem::path directory;
```

The filesystem root used for requests under `routePrefix`.

---

## Example

```cpp
drogular::StaticFileMapping mapping{
    .routePrefix = "/assets",
    .directory = "public"
};
```

The intended mapping is:

```text
/assets/logo.svg  ->  public/logo.svg
```

Application code normally creates mappings through:

```cpp
app.staticFiles("/assets", "public");
```

---

## Related Types

- [`App`](../application/app.md)
- [`ApplicationOptions`](../application/application-options.md)
- [`Router`](../routing/router.md)
- [`StaticFileResolver`](static-file-resolver.md)

## Recursive mappings

Nested asset paths are opt-in to preserve existing mappings:

```cpp
app.staticFiles("/assets", "public", true);
```

The URL `/assets/js/components/menu.js` then maps to
`public/js/components/menu.js`. With the default `false`, only direct
children are served. Path traversal and symlinks escaping the root are rejected;
there is no recursive directory scan at application startup.
