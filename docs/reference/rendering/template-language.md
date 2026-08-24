# Template Language

Drogular templates use a small declarative language built on one shared Expression Engine. Expressions are compiled together with the template and evaluated against request data and lexical bindings during rendering.

---

## Values and expressions

Expressions can read render-context values and lexical bindings, access object members, use arithmetic and comparisons, and combine conditions with `&&`, `||`, and `!`.

```html
@if(user.active && page > 1)
    ...
@endif
```

Collections and ranges are expression values too:

```text
["admin", "moderator"]
[1..10]
[0..<count]
[1..20 step 2]
```

Use `in` and `not in` for membership tests.

```html
@if(user.role in ["admin", "moderator"])
    ...
@endif
```

See [Template Conditional Expressions](conditional-expressions.md) and the [Template Expression Engine](../../architecture/template_expression_engine.md).

---

## Variables and scope

`@let` and `@const` create lexical bindings. A binding is visible from its declaration to the end of the current block. Nested blocks may shadow an outer binding without modifying it.

```html
@const(pageSize = 20)
@let(lastPage = pages.last())
```

`@if`, `@switch` branches, and each `@foreach` iteration use child scopes. Lexical bindings do not mutate the request `RenderContext`.

See [Template Variables](template-variables.md).

---

## Collections and iteration

`@foreach` accepts any iterable expression, including JSON arrays, list literals, ranges, and values produced by expression functions.

```html
@foreach(i in [0..<count])
    {{ i }}
@empty
    No values
@endforeach
```

A loop may use `where`, `@continue`, and `@break`. The `loop` object exposes `index`, `number`, `count`, `first`, `last`, `depth`, and `parent`.

Collection functions provide `count()`, `empty()`, `first()`, `last()`, and `contains(value)`.

See [Template `@foreach`](foreach.md) and [Expression Functions](expression-functions.md).

---

## Control flow

Use `@if` for boolean conditions and `@switch` for value selection.

```html
@switch(project.status)
    @case("active")
        Active
    @case("paused", "blocked")
        Not available
    @default
        Unknown
@endswitch
```

`@switch` has no fallthrough. The first matching case is rendered.

See [Template Conditional Expressions](conditional-expressions.md) and [Template `@switch`](switch.md).

---

## Functions

Expression functions extend the language without adding new template directives. Drogular provides collection methods and applications may register global functions and methods at startup.

```html
{{ t("projects.title") }}
{{ projects.count() }}
@if(projects.contains(currentProject))
    ...
@endif
```

See [Expression Functions](expression-functions.md).

---

## Compilation model

Template control-flow and binding expressions are parsed once during template compilation and stored as Expression AST nodes. Rendering evaluates those compiled expressions against the current `BindingContext` and `ExpressionEnvironment`.

`template_engine::render()` is a compatibility facade over the same compiler and renderer, so there is one control-flow implementation for both legacy and compiled entry points.
