# Expression Collection Functions

Drogular collection functions are methods on expression values. The same API
works for JSON arrays from `RenderContext`, list literals, and integer ranges.

## Supported functions

| Function | Arguments | Result |
| --- | --- | --- |
| `count()` | none | number of elements |
| `empty()` | none | `true` when the collection has no elements |
| `first()` | none | first element, or `null` for an empty collection |
| `last()` | none | last element, or `null` for an empty collection |
| `contains(value)` | one value | membership test |

Examples:

```html
@if(projects.empty())
    <p>No projects.</p>
@endif

@if(projects.count() > 10)
    <p>Large project list.</p>
@endif

@if([1..10].contains(currentPage))
    ...
@endif
```

List and range literals use the same functions:

```text
[1, 3, 5].count()          // 3
[].empty()                 // true
[10..20 step 5].first()   // 10
[10..20 step 5].last()    // 20
[1..<10 step 2].contains(7)
```

`contains(value)` uses the same membership semantics as `in`, so these
expressions are equivalent:

```text
roles.contains(user.role)
user.role in roles
```

Range membership is checked mathematically and does not materialize the range.

## Chaining

Method results are ordinary `ExpressionValue` instances, so member access and
additional collection calls may continue from the result:

```text
projects.first().name
projects.first().roles.contains("Admin")
```

For empty collections, `first()` and `last()` return `null`, which composes with
normal null/truthiness behavior.

## Invalid calls

Collection methods have fixed arity. Calls with the wrong number of arguments,
unknown methods, or collection methods invoked on a non-iterable value evaluate
to `null` rather than throwing during template rendering.

## Architecture

Methods are represented by `MethodCallExpression` in the expression AST. The
evaluator resolves method names through `ExpressionFunctionRegistry`; built-in
collection functions live outside the evaluator itself. This keeps parser and
evaluator changes unnecessary when additional built-in functions are added.
