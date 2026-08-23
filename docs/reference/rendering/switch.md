# Template `@switch`

`@switch` selects exactly one branch by comparing an expression value with one
or more `@case` expressions. There is no fallthrough and no `@break` is needed.

## Basic syntax

```html
@switch(status)
    @case("Draft")
        <span>Draft</span>
    @case("Published")
        <span>Published</span>
    @default
        <span>Unknown</span>
@endswitch
```

The first matching case is rendered. If no case matches, `@default` is rendered
when present. If there is no matching case and no default branch, the directive
renders nothing.

## Multiple values

A case may contain several expressions:

```html
@switch(status)
    @case("Draft", "Pending")
        <span>Waiting</span>
    @case("Published")
        <span>Public</span>
@endswitch
```

The branch matches when the switch value equals any case value. Equality uses
the same `ExpressionValue` semantics as the expression `==` operator.

## Expression values

Both the switch selector and case values use the common Expression Engine:

```html
@const(lastPage = pages.count() - 1)

@switch(page)
    @case(0)
        First
    @case(lastPage)
        Last
    @default
        Middle
@endswitch
```

Custom expression functions are supported as well:

```html
@switch(user.role)
    @case("Admin")
        {{ t("role.admin") }}
    @case("User", "Guest")
        {{ t("role.standard") }}
@endswitch
```

## Lexical scope

The selected branch renders in a child `BindingContext`. Bindings declared in a
case or default branch do not leak outside the switch:

```html
@switch(status)
    @case("Draft")
        @let(label = "draft")
        {{ label }}
@endswitch

{{ label }}
```

The final interpolation does not see the branch-local `label` binding.

Nested switches are supported and follow the same lexical-scope rules.

## No fallthrough

Drogular deliberately does not implement C/C++-style fallthrough. A matching
case renders once and completes the switch. `@break` remains a loop-control
directive and is not required by `@switch`.

## Diagnostics

| Code | Meaning |
| --- | --- |
| `DGL-TPL-025` | `@case` outside `@switch`, or invalid content before a switch branch |
| `DGL-TPL-026` | `@default` outside `@switch` |
| `DGL-TPL-027` | Duplicate `@default` |
| `DGL-TPL-028` | Invalid `@switch(...)` expression |
| `DGL-TPL-029` | Invalid `@case(...)` expression |
| `DGL-TPL-030` | Missing `@endswitch` |
| `DGL-TPL-031` | Unexpected `@endswitch` |

## See also

- [Template Conditional Expressions](conditional-expressions.md)
- [Template Variables](template-variables.md)
- [Expression Collection Functions](expression-functions.md)
