# Template `@foreach`

The `@foreach` directive renders a block once for each selected item in a collection.

## Basic loop

```html
@foreach(todo in todos)
    <li>{{ todo.title }}</li>
@endforeach
```

The existing `@foreach(variable in collection)` form remains the basic syntax.

## Filtering with `where`

A loop can filter its collection before rendering:

```html
@foreach(todo in todos where !todo.completed)
    <li>{{ todo.title }}</li>
@endforeach
```

The `where` condition uses the same expression language as `@if(...)`, including comparisons, logical operators, literals, dotted paths, and grouping.

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

Example:

```html
@foreach(item in items)
    {{ loop.number }}/{{ loop.count }} — {{ item.name }}
@endforeach
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

- the collection is missing or cannot be resolved as an array;
- the collection is empty;
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
