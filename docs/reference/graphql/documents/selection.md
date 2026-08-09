# `gql::Selection`

**Namespace:** `drogular::gql`  
**Header:** `<drogular/graphql.hpp>`  
**Kind:** Class

## Purpose

`Selection` represents a field selection or fragment spread in a generated GraphQL document.

## Construction Helpers

```cpp
Selection field(std::string name);
Selection field(std::string name, std::vector<Selection> children);
Selection spread(std::string name);
```

## Public API

```cpp
Selection& alias(std::string alias);
Selection& arg(std::string name, Value value);
Selection& children(std::vector<Selection> children);

std::string toString(unsigned int indent = 2) const;

const std::string& name() const;
SelectionKind kind() const;
const std::optional<std::string>& alias() const;
const std::vector<Argument>& arguments() const;
const std::vector<Selection>& children() const;
```

`SelectionKind` is either `Field` or `FragmentSpread`.

Arguments preserve insertion order. `children()` replaces the complete existing child vector.

## Example

```cpp
auto selection = drogular::gql::field("user")
    .alias("profile")
    .arg("id", drogular::gql::variable("userId"))
    .children({
        drogular::gql::field("id"),
        drogular::gql::field("name")
    });
```

## See Also

- [`gql::Value`](value.md)
- [`gql::Fragment`](fragment.md)
- [`gql::Query`](query.md)
