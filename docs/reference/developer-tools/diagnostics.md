# `Diagnostics`

**Namespace:** `drogular`  
**Header:** `<drogular/diagnostics.hpp>`  
**Kind:** Diagnostic collection class

## Purpose

`Diagnostics` collects structured informational, warning, and error entries.

Developer Tools surface these entries through [`ApplicationInspection`](application-inspection.md), while the same primitives are also used by framework subsystems such as component and template diagnostics.

## Supporting Types

```cpp
enum class DiagnosticSeverity {
    Info,
    Warning,
    Error
};
```

```cpp
struct SourceLocation {
    std::string source;
    std::size_t position = 0;
    std::size_t line = 1;
    std::size_t column = 1;
};
```

```cpp
struct Diagnostic {
    std::string code;
    DiagnosticSeverity severity =
        DiagnosticSeverity::Error;
    std::string message;
    SourceLocation location;
};
```

## Construction

```cpp
Diagnostics();

explicit Diagnostics(
    std::string_view sourceText,
    std::string sourceName = {}
);
```

Providing source text enables position-based diagnostics to compute line and column information.

`Diagnostics` stores its own copy of `sourceText`, so the input view does not need to outlive the object.

## Adding Diagnostics

```cpp
void add(Diagnostic diagnostic);
```

Or use severity-specific helpers:

```cpp
void info(...);
void warning(...);
void error(...);
```

Each severity has an overload accepting a complete `SourceLocation` and another accepting a byte position in the configured source text.

Only `Error` entries are copied into the dedicated `errors()` collection.

## Querying State

```cpp
bool hasErrors() const;
bool empty() const;
bool valid() const;

const std::vector<Diagnostic>& entries() const;
const std::vector<Diagnostic>& errors() const;
```

`valid()` is equivalent to `!hasErrors()`; warnings and informational entries do not make a collection invalid.

## `locationAt()`

```cpp
SourceLocation locationAt(
    std::size_t position
) const;
```

Converts a byte position into source, line, and column information.

Positions beyond the source length are clamped to the end of the source.

Line and column counting starts at `1`. Newline characters increment the line and reset the column to `1`.

## `clear()`

```cpp
void clear();
```

Clears diagnostic entries and the dedicated error list. The configured source text and source name remain unchanged.

## Component Alias

```cpp
using ComponentDiagnostics = Diagnostics;
```

The alias is declared in `<drogular/component_diagnostics.hpp>`.

## Thread Safety

`Diagnostics` has no internal synchronization. Do not mutate the same collection concurrently without external synchronization.

## Related Types

- [`ApplicationInspection`](application-inspection.md)
- [`ComponentRegistry`](../components/component-registry.md)
