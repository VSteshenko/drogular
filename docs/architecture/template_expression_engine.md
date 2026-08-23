# Template Expression Engine

Drogular template directives share one expression engine. The engine is kept
separate from the template parser so control-flow directives do not implement
their own expression grammars.

## Pipeline

```text
expression source
      │
      ▼
    Lexer
      │
      ▼
Expression Parser
      │
      ▼
 Expression AST
      │
      ▼
   Evaluator
      │
      ▼
ExpressionValue
```

The template parser owns directive structure (`@if`, `@foreach`, and future
control-flow nodes). It passes expression source to the expression engine and
uses the resulting value or diagnostic.

## Module layout

The engine lives independently under `template/expression`:

```text
include/drogular/template/expression/
    value.hpp
    ast.hpp
    parser.hpp
    evaluator.hpp
    expression.hpp

src/template/expression/
    parser.cpp
    evaluator.cpp
```

`<drogular/template_expression.hpp>` remains as a compatibility umbrella. New
code may include the narrower expression headers directly.

## ExpressionValue

`template_expression::ExpressionValue` is the common runtime value abstraction.
It supports:

- null/missing values;
- booleans;
- numbers;
- strings;
- existing `Json::Value` objects and arrays;
- expression-owned arrays;
- integer ranges.

Keeping JSON as a supported value allows existing `RenderContext` data and
dotted JSON paths to pass through the engine without an intermediate model.
Expression-owned arrays and ranges provide the foundation for iterable template
expressions without coupling them to `Json::Value`.

## Iterable values

`ExpressionValue::iterable()` provides one indexed view over all collection
forms currently understood by the engine:

- `Json::Value` arrays;
- expression list literals;
- integer ranges.

`ExpressionIterable` exposes `size()`, `empty()`, and `at(index)`. Range values
are generated on demand rather than materialized, so a loop over a range does
not first allocate an intermediate array. The same iterable abstraction is used
by membership operations and `@foreach`.

The evaluated element is bound back into the child `RenderContext` without
losing its expression type. This is important for nested expression-owned
collections and is also the storage model future `@let` / `@const` bindings can
reuse.

## Expression AST

The AST contains:

- `LiteralExpression`;
- `VariableExpression`;
- `UnaryExpression`;
- `BinaryExpression`;
- `ListExpression`;
- `RangeExpression`.

Unary and binary nodes carry explicit operator enums rather than source text.
The evaluator therefore works on parsed semantics and does not need to inspect
the original expression string.

The AST is immutable after parsing and may be reused for multiple evaluations
against different `RenderContext` instances.

## Arithmetic expressions

Arithmetic is part of the common expression grammar rather than a special
feature of ranges. The engine supports:

```text
+  -  *  /
```

as well as unary negation:

```text
-count
```

Normal arithmetic precedence applies: unary operators, multiplication/division,
addition/subtraction, comparisons and membership, `&&`, then `||`.

This means range bounds and steps may be arbitrary expressions:

```text
[page * size..(page + 1) * size - 1]
[start..<total step stride * 2]
[(first + 1)..last]
```

The same arithmetic expressions are also available to `@if`, `where`, and
future expression consumers.

## List literals

Lists may contain arbitrary expressions:

```text
[]
[1, 3, 5, 7]
[user.id, page + 1, total * 2]
[[1, 2], [3, 4]]
```

A trailing comma is accepted.

## Range literals

Ranges are integer-valued iterable expressions.

Inclusive upper bound:

```text
[1..10]
```

Exclusive upper bound:

```text
[0..<10]
```

Explicit step:

```text
[1..10 step 2]
[10..1 step -2]
```

Without an explicit step, the evaluator selects `1` for ascending ranges and
`-1` for descending ranges.

Bounds and step expressions are evaluated against the current `RenderContext`:

```text
[start..end]
[page * size..(page + 1) * size - 1]
[0..<itemCount step stride]
```

Range values require integral bounds and an integral, non-zero step. An
explicit step whose direction cannot reach the end bound evaluates to null,
which prevents a non-terminating range.

## Membership operators

The expression engine supports membership tests at comparison precedence:

```text
value in collection
value not in collection
```

Membership works with expression-owned list literals, integer ranges, and
`Json::Value` arrays resolved from `RenderContext`:

```text
role in ["Admin", "Moderator"]
page in [1..10]
index not in [0..<reservedCount]
user.id in allowedUserIds
```

Range membership respects both the upper-bound mode and `step`. It is checked
mathematically and does not materialize the range, so membership in a large
range does not allocate an intermediate array.

The right operand must be an iterable collection supported by the expression
engine. Membership against another value type evaluates to `false`; `not in`
is its logical inverse. `in` and `not` remain ordinary identifiers outside the
binary-operator position, so existing context keys with those names continue
to resolve normally.

## RenderContext resolution

`template_expression::resolve()` resolves primitive context values and dotted
JSON paths such as:

```text
user.profile.active
```

Missing values produce a null `ExpressionValue`. This preserves existing
conditional behavior, including explicit comparisons such as:

```html
@if(user == null)
```

## Compatibility facade

Existing template-runtime entry points remain valid:

```cpp
validateConditionExpression(...)
evaluateCondition(...)
```

They delegate to the expression engine. This keeps the current public template
API source-compatible while allowing future directives to consume parsed
expressions directly.

## `@foreach` integration

The collection side of `@foreach` is now a full expression rather than a
RenderContext key:

```html
@foreach(i in [1..10])
    {{ i }}
@endforeach

@foreach(i in [0..<count step 2])
    {{ i }}
@endforeach

@foreach(role in ["Admin", "Moderator"])
    {{ role }}
@endforeach
```

Existing context-backed forms remain unchanged because identifiers and dotted
paths are expressions too. The optional `where` clause is evaluated after each
source element is bound and before loop metadata is calculated.

## Extension path

The expression engine now provides the value and iterable models needed for
the next language steps:

1. collection functions such as `count()`, `empty()`, `contains()`, `first()`, and `last()`;
2. `@let` bindings;
3. compile-time `@const` bindings;
4. `@switch` / `@case`.

Those features belong in the expression engine rather than in individual
template directives whenever they represent values or operators.
