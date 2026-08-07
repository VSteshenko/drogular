# LocaleSupport

**Namespace:** `drogular`  
**Header:** `<drogular/locale_support.hpp>`  
**Kind:** Static utility class

## Purpose

`LocaleSupport` resolves the active locale from the current request.

It only selects a locale. It does not perform translation.

---

## Synopsis

```cpp
class LocaleSupport {
public:
    static std::string current(
        RenderContext& context
    );

    static std::string current(
        RenderContext& context,
        const std::string& defaultLocale
    );
};
```

---

## Public API

### `current(context)`

```cpp
static std::string current(
    RenderContext& context
);
```

Returns the supported locale stored in the `lang` request cookie.

When the cookie is missing or unsupported, the method returns `"en"`.

### `current(context, defaultLocale)`

```cpp
static std::string current(
    RenderContext& context,
    const std::string& defaultLocale
);
```

Returns the supported locale stored in the `lang` request cookie.

When the cookie is missing or unsupported, the supplied `defaultLocale` is returned.

---

## Behavior

The current implementation recognizes exactly two cookie values:

- `"en"`
- `"de"`

Any other value is treated as unsupported.

`RenderContext::cookie()` returns no value when there is no request, the cookie is absent, or the cookie value is empty. In all of these cases `LocaleSupport` returns the configured default.

Locale selection is intentionally separate from translation lookup.

---

## Example

```cpp
const auto locale =
    drogular::LocaleSupport::current(context);
```

Use a custom default:

```cpp
const auto locale =
    drogular::LocaleSupport::current(
        context,
        "de"
    );
```

---

## Related Types

- [`TranslationSupport`](translation-support.md)
- [`TranslationProvider`](translation-provider.md)
- [`RenderContext`](../rendering/render-context.md)

## See Also

- [Cookbook: Localization](../../cookbook/localization.md)
