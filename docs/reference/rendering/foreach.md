# Template `@foreach`

The `@foreach` directive renders a block once for each selected item produced by an iterable expression.

## Basic loop

```html
@foreach(todo in todos)
    <li>{{ todo.title }}</li>
@endforeach
```

The source after `in` is a full template expression. Existing context-backed
collections therefore remain unchanged, while list and range expressions can
be used directly:

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

Supported iterable results are JSON arrays, expression list literals, and
integer ranges. Range values are generated on demand rather than materialized
into an intermediate array.

## Filtering with `where`

A loop can filter its collection before rendering:

```html
@foreach(todo in todos where !todo.completed)
    <li>{{ todo.title }}</li>
@endforeach
```

The `where` condition uses the same expression language as `@if(...)`, including
arithmetic, comparisons, membership (`in` / `not in`), logical operators,
literals, dotted paths, lists, ranges, and grouping.

Filtering happens before loop metadata is calculated. Therefore `loop.count`, `loop.index`, `loop.first`, and `loop.last` describe the filtered sequence.

## Loop metadata

Each iteration exposes a `loop` object:

| Property | Meaning |
| --- | --- |
| `loop.index` | Zero-based index of the current selected item |
| `loop.number` | One-based iteration number |
| `loop.count` | Number of selected items |
| `loop.first` | `true` for the first selected item |
| `loop.last` | `true` for the last selected item |
| `loop.depth` | Zero-based nesting depth; the outermost loop has depth `0` |
| `loop.parent` | Metadata of the nearest outer loop, or `null` for the outermost loop |

Example:

```html
@foreach(item in items)
    {{ loop.number }}/{{ loop.count }} — {{ item.name }}
@endforeach
```

## Nested loops

Each nested `@foreach` gets its own `loop` object. The current loop metadata
shadows the outer loop metadata, while `loop.parent` provides access to the
nearest outer loop. Parents can be chained for deeper nesting.

```html
@foreach(category in categories)
    <h2>{{ loop.number }}. {{ category.name }}</h2>

    @foreach(product in category.products)
        <p>
            {{ loop.parent.number }}.{{ loop.number }}
            {{ product.name }}
        </p>
    @endforeach
@endforeach
```

`loop.depth` starts at `0` for the outermost loop and increases by one for
each nested `@foreach`:

```html
@foreach(category in categories)
    {{ loop.depth }}

    @foreach(product in category.products)
        {{ loop.depth }}

        @foreach(tag in product.tags)
            {{ loop.depth }}
            {{ loop.parent.parent.number }}
        @endforeach
    @endforeach
@endforeach
```

The three nesting levels above have depths `0`, `1`, and `2`. On the
outermost loop `loop.parent` is `null`, so it can be tested directly:

```html
@if(loop.parent)
    Nested loop
@endif
```

## Empty branch

Use `@empty` to render fallback content when the loop has no selected items:

```html
@foreach(todo in todos where !todo.completed)
    <li>{{ todo.title }}</li>
@empty
    <li>No open todos.</li>
@endforeach
```

The empty branch is used when:

- the source expression is missing, evaluates to a non-iterable value, or cannot be resolved;
- the resulting iterable is empty;
- a `where` condition filters out every item.

`@empty` belongs to the nearest `@foreach` block.

## `@continue`

`@continue` stops rendering the current iteration and proceeds with the next selected item:

```html
@foreach(todo in todos)
    @if(todo.hidden)
        @continue
    @endif

    <li>{{ todo.title }}</li>
@endforeach
```

Content rendered before `@continue` is preserved; content after it in the current iteration is skipped.

## `@break`

`@break` stops the current loop:

```html
@foreach(result in results)
    @if(loop.number > 10)
        @break
    @endif

    <li>{{ result.title }}</li>
@endforeach
```

Content rendered before `@break` is preserved.

For nested loops, `@break` and `@continue` affect only the nearest active `@foreach`:

```html
@foreach(row in rows)
    @foreach(cell in row.cells)
        @if(cell.hidden)
            @continue
        @endif

        @if(cell.stopRow)
            @break
        @endif

        {{ cell.value }}
    @endforeach
@endforeach
```

In this example `@break` stops only the inner cell loop. The outer row loop continues.

## Diagnostics

Template compilation reports invalid loop directives through `TemplateDiagnostics`:

| Code | Meaning |
| --- | --- |
| `DGL-TPL-007` | Invalid `@foreach(...)` expression |
| `DGL-TPL-008` | Invalid `where` condition |
| `DGL-TPL-009` | Unexpected `@empty` |
| `DGL-TPL-010` | `@break` used outside an active `@foreach` scope |
| `DGL-TPL-011` | `@continue` used outside an active `@foreach` scope |

A missing `@endforeach` continues to use `DGL-TPL-005`.

## See Also

- [Template Conditional Expressions](conditional-expressions.md)
- [`template_compiler::CompiledTemplate`](compiled-template.md)
- [`RenderContext`](render-context.md)
