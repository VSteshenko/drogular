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

## Application functions

Applications can extend the expression language during startup without changing
the parser or evaluator.

Global functions are registered through `App::expressionFunction`:

```cpp
app.expressionFunction(
    "t",
    [](std::span<const ExpressionValue> args,
       const BindingContext& context) -> ExpressionValue {
        if (args.size() != 1 || !args[0].string()) {
            return {};
        }

        // Resolve an application localization service from the base
        // RenderContext and translate using the current request locale.
        auto localizer = context.renderContext().requireService<MyLocalizer>();
        return ExpressionValue(localizer->translate(*args[0].string()));
    }
);
```

The function is then available everywhere an expression is accepted:

```html
<h1>{{ t("projects.title") }}</h1>

@let(emptyMessage = t("projects.empty"))

@if(projects.empty())
    <p>{{ emptyMessage }}</p>
@endif
```

Receiver methods are registered through `App::expressionMethod`:

```cpp
app.expressionMethod(
    "currency",
    [](const ExpressionValue& self,
       std::span<const ExpressionValue> args,
       const BindingContext&) -> ExpressionValue {
        // ...
    }
);
```

and used as normal expression methods:

```text
price.currency("EUR")
```

Callbacks receive the current `BindingContext`, so they can read lexical
`@let` / `@const` values and reach request/application services through
`context.renderContext()`. Functions should treat bindings as read-only.

## Registration lifecycle

Expression extensions are intended to be registered during application startup.
`App::run()` freezes the application registry before the server begins serving
requests. Attempts to register another function after the registry is frozen are
rejected.

Built-in names such as `count`, `empty`, `first`, `last`, and `contains` cannot
be replaced through the `App` facade. Duplicate application names are also
rejected.

User callback exceptions are contained by the expression environment and the
call evaluates to `null` instead of aborting template rendering.

## Runtime architecture

Expression evaluation uses an `ExpressionEnvironment` containing the lexical
`BindingContext` plus the application function registry. Resolution order is:

1. application functions/methods;
2. built-in functions/methods;
3. `null` when no callable exists.

Global calls are represented by `CallExpression`; receiver calls continue to
use `MethodCallExpression`. Function results are ordinary `ExpressionValue`
instances, so chaining works naturally.
