# Template Variables

Drogular templates support lexical bindings with `@let` and `@const`. Bindings are evaluated at render time by the shared Template Expression Engine and live in `BindingContext`, not in the application-owned `RenderContext`.

## `@let`

Use `@let` to define a lexical binding:

```html
@let(total = projects.count())
<p>{{ total }}</p>
```

The right-hand side is a normal template expression and may use arithmetic, member access, collection functions, lists, ranges, application expression functions, and values inherited from `RenderContext`.

The current template language defines bindings but does not provide an assignment directive for changing an existing binding after declaration.

## `@const`

Use `@const` for a lexical binding recorded with constant mutability:

```html
@const(PageSize = 20)
<p>Page size: {{ PageSize }}</p>
```

`@const` uses the same expression grammar and scope rules as `@let`.

## Scope

Bindings are visible from their declaration to the end of the current lexical scope. `@if` branches, `@switch` branches, and each `@foreach` iteration create child scopes.

A child scope may shadow an outer binding without modifying it:

```html
@let(value = 1)
@if(true)
    @let(value = 2)
    {{ value }}
@endif
{{ value }}
```

The inner expression renders `2`; after the block the outer binding still resolves to `1`. A duplicate binding in the same scope is rejected during template compilation. Bindings shadow values with the same name from `RenderContext` without modifying the context itself.

## Compilation and diagnostics

Binding expressions are parsed when the template is compiled and stored in the Template AST. They are evaluated when the template renders.

| Code | Meaning |
|---|---|
| `DGL-TPL-020` | invalid `@let` expression |
| `DGL-TPL-021` | duplicate binding in the same scope |
| `DGL-TPL-022` | invalid `@let` binding identifier |
| `DGL-TPL-023` | invalid `@const` expression |
| `DGL-TPL-024` | invalid `@const` binding identifier |

## Related Documentation

- [Template Language](template-language.md)
- [Template Conditional Expressions](conditional-expressions.md)
- [Template `@foreach`](foreach.md)
- [Template `@switch`](switch.md)
- [Expression Functions](expression-functions.md)
- [Template Expression Engine](../../architecture/template_expression_engine.md)
