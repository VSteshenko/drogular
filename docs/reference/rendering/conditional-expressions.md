# Template Conditional Expressions

Drogular templates support conditional expressions in `@if(...)` directives.

Simple truthy checks remain valid:

```html
@if(isAdmin)
    <a href="/admin">Admin</a>
@endif
```

Conditions can also combine comparisons, logical operators, literals, dotted JSON paths, and parentheses:

```html
@if(user.role == "admin" && user.active)
    <a href="/admin">Admin</a>
@endif

@if(page > 1 && page <= totalPages)
    <a href="?page={{ previousPage }}">Previous</a>
@endif

@if(!(disabled || loading))
    <button type="submit">Save</button>
@endif
```

## Supported Operators

| Category | Operators           |
| --- |---------------------|
| Arithmetic | `+`, `-`, `*`, `/` |
| Equality | `==`, `!=`          |
| Relational | `<`, `<=`, `>`, `>=` |
| Membership | `in`, `not in` |
| Logical | `&&`, `\|\|` |
| Unary | `!`, unary `-`       |
| Grouping | `(`, `)`            |

Operator precedence, from highest to lowest, is:

1. parentheses and unary `!` / unary `-`
2. `*` and `/`
3. `+` and `-`
4. comparison and membership operators
5. `&&`
6. `||`

Use parentheses when they make the intended condition clearer.

## Values and Literals

Conditions can read values available in `RenderContext`:

```html
@if(isAuthenticated)
    ...
@endif

@if(page >= 2)
    ...
@endif
```

Dotted paths can address nested `Json::Value` fields:

```html
@if(user.profile.active && user.role == "admin")
    ...
@endif
```

The following literals are supported:

- booleans: `true`, `false`
- null: `null`
- numbers: `10`, `-2`, `3.5`
- strings: `"active"`, `'admin'`

Both single-quoted and double-quoted string literals are accepted.

List and range expressions can be used directly in conditions:

```html
@if(user.role in ["Admin", "Moderator"])
    ...
@endif

@if(page in [1..10])
    ...
@endif

@if(index not in [0..<reservedCount])
    ...
@endif
```

Membership also works with JSON arrays supplied through `RenderContext`.

## Truthy Conditions

A condition does not need an explicit comparison. Existing templates using a single value continue to work:

```html
@if(showNavigation)
    ...
@endif
```

Missing values resolve as null/falsy values. This also allows an explicit null check:

```html
@if(user == null)
    ...
@endif
```

## Diagnostics

`compileWithDiagnostics()` validates the syntax of every `@if(...)` expression while compiling the template.

For example:

```html
@if(page >)
    ...
@endif
```

produces an error similar to:

```text
DGL-TPL-006: Invalid @if expression: Expected value
```

The diagnostic location points to the position inside the `@if` expression where parsing failed and includes the source name, line, and column when a source name was supplied to `compileWithDiagnostics()`.

Malformed directives that do not contain a closing `)` are reported as:

```text
DGL-TPL-006: Invalid @if expression: Missing closing ')'
```

Other syntax errors include missing closing grouping parentheses, unterminated string literals, invalid number literals, and unexpected tokens.

Low-level calls to `evaluateCondition()` remain safe: an invalid expression evaluates to `false`. Normal template rendering should rely on compilation diagnostics to detect syntax errors before rendering.

## Examples

### Role and state

```html
@if(user.role == "admin" && user.active)
    <AdminNavigation />
@else
    <UserNavigation />
@endif
```

### Pagination

```html
@if(page > 1)
    <a href="?page={{ previousPage }}">Previous</a>
@endif

@if(page < totalPages)
    <a href="?page={{ nextPage }}">Next</a>
@endif
```

### Grouped logic

```html
@if(isOwner || (isAdmin && !readOnly))
    <button type="submit">Edit</button>
@endif
```

## Architecture

`@if(...)` does not implement a separate condition parser. Conditions are parsed into the shared Template Expression AST and evaluated through `template_expression::ExpressionValue`. The compatibility functions `validateConditionExpression()` and `evaluateCondition()` delegate to that engine. 

See [Template Expression Engine](../../architecture/template_expression_engine.md).

## Related APIs

- [`CompiledTemplate`](compiled-template.md)
- [`RenderContext`](render-context.md)
- [`TemplateRenderable`](template-renderable.md)
