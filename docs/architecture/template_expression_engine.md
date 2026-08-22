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

## ExpressionValue

`template_expression::ExpressionValue` is the common runtime value abstraction.
The initial implementation supports:

- null/missing values;
- booleans;
- numbers;
- strings;
- existing `Json::Value` objects and arrays.

Keeping JSON as a supported value allows existing `RenderContext` data and
dotted JSON paths to pass through the engine without an intermediate model.
The abstraction is intentionally extensible so list and range values can be
added without changing template directives.

## Expression AST

The initial AST contains:

- `LiteralExpression`;
- `VariableExpression`;
- `UnaryExpression`;
- `BinaryExpression`.

Unary and binary nodes carry explicit operator enums rather than source text.
The evaluator therefore works on parsed semantics and does not need to inspect
the original expression string.

The AST is immutable after parsing and may be reused for multiple evaluations
against different `RenderContext` instances.

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

## Extension path

The expression engine is the foundation for the next template-language steps:

1. list literals;
2. inclusive and exclusive ranges;
3. range steps;
4. `in` / `not in`;
5. `@foreach` over arbitrary iterable expressions;
6. `@let` bindings;
7. `@switch` / `@case`.

Those features belong in the expression engine rather than in individual
template directives whenever they represent values or operators.
