# `gql::Value`

**Namespace:** `drogular::gql`  
**Header:** `<drogular/graphql.hpp>`  
**Kind:** Class and factory functions

## Purpose

`Value` stores already serialized GraphQL literal text used by field arguments.

## Factories

```cpp
Value string(std::string value);
Value intValue(int value);
Value boolValue(bool value);
Value raw(std::string value);
Value variable(std::string name);
```

`variable("id")` serializes as `$id`.

`raw()` performs no validation or escaping and should be used only when the caller already has valid GraphQL syntax.

`string()` currently escapes double quotes and backslashes. It does not implement full GraphQL string escaping for all control characters.

## Access

```cpp
const std::string& toString() const;
```

## See Also

- [`gql::Selection`](selection.md)
