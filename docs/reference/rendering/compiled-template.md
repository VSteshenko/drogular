# `template_compiler::CompiledTemplate`

**Namespace:** `drogular::template_compiler`  
**Header:** `<drogular/compiled_template.hpp>`  
**Kind:** Class and compilation functions

## Purpose

`CompiledTemplate` represents template source compiled into Drogular's template AST and ready to render against a `RenderContext`.

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

Evaluates the compiled template against the supplied `RenderContext` and returns the resulting text.

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

A `CompiledTemplate` owns its compiled AST. Rendering reads that AST and uses the supplied `RenderContext` for request-specific values.

The API itself does not promise synchronization for concurrent access to a shared `RenderContext`; normal rendering uses request-local contexts.

## Related Types

- [`TemplateRenderable`](template-renderable.md)
- [`RenderContext`](render-context.md)
- `TemplateDiagnostics`
