# `StaticFileResolver`

**Namespace:** `drogular`  
**Header:** `<drogular/static_file_resolver.hpp>`  
**Kind:** Class

## Purpose

`StaticFileResolver` resolves a requested file path relative to a configured root directory while preventing the result from escaping that root.

It is the path-safety layer used by registered static-file routes.

---

## Construction

```cpp
explicit StaticFileResolver(
    std::filesystem::path root
);
```

The constructor stores:

```cpp
std::filesystem::weakly_canonical(root)
```

as the resolver root.

---

## Public API

### `resolve()`

```cpp
std::optional<std::filesystem::path> resolve(
    const std::string& filePath
) const;
```

The method:

1. combines `root` and `filePath`;
2. applies `std::filesystem::weakly_canonical()`;
3. verifies that the resulting path remains inside the configured root;
4. verifies that the path exists;
5. verifies that it is a regular file.

Returns the canonical path when all checks succeed.

Returns `std::nullopt` when the requested path:

- escapes the configured root;
- does not exist;
- is not a regular file.

```cpp
drogular::StaticFileResolver resolver("public");

const auto file = resolver.resolve("images/logo.svg");
```

---

## Path Traversal Protection

A request such as:

```text
../secret.txt
```

is canonicalized before the root-prefix check and is rejected when the result is outside the configured root.

The check compares normalized path strings and includes the platform-preferred directory separator when testing the root prefix.

Filesystem canonicalization and metadata errors are not converted into `std::nullopt`; exceptions from `std::filesystem` may propagate.

---

## Related Types

- [`StaticFileMapping`](static-file-mapping.md)
- [`StaticFileResponse`](static-file-response.md)
- [`Router`](../routing/router.md)
