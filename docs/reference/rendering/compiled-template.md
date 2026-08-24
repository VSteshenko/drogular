# `template_compiler::CompiledTemplate`

**Namespace:** `drogular::template_compiler`  
**Header:** `<drogular/compiled_template.hpp>`  
**Kind:** Class and compilation functions

## Purpose

`CompiledTemplate` represents template source compiled into Drogular's template AST, including precompiled Expression AST for control-flow and binding directives, and ready to render against a `RenderContext`.

Most applications do not need to compile templates directly because `TemplateRenderable` performs compilation and caching automatically. The low-level API is useful when integrating directly with the template compiler.

## `CompiledTemplate`

```cpp
class CompiledTemplate {
public:
    explicit CompiledTemplate(std::vector<NodePtr> nodes);

    std::string render(RenderContext& context) const;
};
```

### `render()`

Evaluates the compiled template against the supplied `RenderContext` and returns the resulting text. Directive expressions are evaluated from the immutable AST produced during compilation; `render()` does not reparse `@if`, `@foreach`, `@let`, `@const`, `@switch`, or `@case` expressions.

## `compile()`

```cpp
CompiledTemplate compile(std::string_view html);
```

Compiles template text into a reusable `CompiledTemplate`.

## `CompileResult`

```cpp
struct CompileResult {
    CompiledTemplate compiledTemplate;
    TemplateDiagnostics diagnostics;

    bool valid() const;
};
```

`valid()` returns the validity state reported by the associated diagnostics collection.

## `compileWithDiagnostics()`

```cpp
CompileResult compileWithDiagnostics(
    std::string_view html,
    std::string sourceName = {}
);
```

Compiles template text and returns the compiled result together with template diagnostics.

## Conditional-expression diagnostics

Template compilation validates the syntax of `@if(...)` expressions. Invalid expressions are reported through `TemplateDiagnostics` using `DGL-TPL-006`, with a source position inside the condition where parsing failed.

For example, `@if(page >)` reports `Invalid @if expression: Expected value` instead of silently treating the malformed condition as a normal false branch.

See [Template Conditional Expressions](conditional-expressions.md) for the supported operators, literals, precedence rules, and diagnostic behavior.

## Loop directives

Compiled templates support expression-based `@foreach(...)` sources, filtering with `where`, `loop.*` metadata, an `@empty` fallback branch, and `@continue` / `@break` control directives. Loop control is scoped to the nearest active `@foreach`.

See [Template `@foreach`](foreach.md) for syntax, semantics, and diagnostics.

## Lifetime and Thread Safety

A `CompiledTemplate` owns both the Template AST and the immutable Expression AST referenced by directive nodes. Rendering reads those ASTs and uses the supplied `RenderContext` / lexical bindings for request-specific values.

The API itself does not promise synchronization for concurrent access to a shared `RenderContext`; normal rendering uses request-local contexts.

## Local variables

Compiled templates support lexical `@let(name = expression)` and `@const(name = expression)` bindings. Bindings
are evaluated through the shared Expression Engine, remain visible to later
nodes in the same block, and may be shadowed in nested `@if` or `@foreach`
scopes without mutating the underlying `RenderContext`.

See [Template Variables](template-variables.md).

## Switch directives

Compiled templates support `@switch(expression)`, multiple `@case(...)` values, and an optional `@default` branch. Switches have no fallthrough; the first matching case renders in its own lexical binding scope.

See [Template `@switch`](switch.md).

## Compatibility renderer

`template_engine::render()` is a compatibility facade over the same tokenizer,
parser, `CompiledTemplate`, and render runtime. Drogular no longer maintains a
second string-scanning implementation of `@if`, `@foreach`, or loop control.
Malformed templates detected by the compiler are returned unchanged by the
compatibility facade, preserving the historical `template_engine::render()`
contract.

## Related Types

- [`TemplateRenderable`](template-renderable.md)
- [`RenderContext`](render-context.md)
- `TemplateDiagnostics`
