# TranslationSupport

**Namespace:** `drogular`  
**Header:** `<drogular/translation_support.hpp>`  
**Kind:** Static utility class

## Purpose

`TranslationSupport` translates keys through the `TranslationProvider` registered in a `RenderContext`.

It can use either the locale resolved by `LocaleSupport` or an explicit locale supplied by the caller.

---

## Synopsis

```cpp
class TranslationSupport {
public:
    static std::string translate(
        RenderContext& context,
        const std::string& key
    );

    static std::string translate(
        RenderContext& context,
        const std::string& locale,
        const std::string& key
    );
};
```

---

## Public API

### `translate(context, key)`

```cpp
static std::string translate(
    RenderContext& context,
    const std::string& key
);
```

Resolves the current locale through `LocaleSupport::current(context)` and translates `key` with the registered provider.

The default locale is therefore `"en"` when no supported locale is present in the request.

### `translate(context, locale, key)`

```cpp
static std::string translate(
    RenderContext& context,
    const std::string& locale,
    const std::string& key
);
```

Translates `key` using the provided locale without consulting `LocaleSupport`.

---

## Behavior

Both overloads resolve `TranslationProvider` through `RenderContext::service<TranslationProvider>()`.

If no provider is registered, the key itself is returned:

```cpp
const auto value =
    drogular::TranslationSupport::translate(
        context,
        "users.title"
    );
```

Without a provider, `value` is `"users.title"`.

Provider exceptions are not caught by `TranslationSupport`.

---

## Example

Use the current request locale:

```cpp
const auto title =
    drogular::TranslationSupport::translate(
        context,
        "users.title"
    );
```

Use an explicit locale:

```cpp
const auto title =
    drogular::TranslationSupport::translate(
        context,
        "de",
        "users.title"
    );
```

Pages normally use `RenderContext::translate()` instead of calling `TranslationSupport` directly.

---

## Related Types

- [`TranslationProvider`](translation-provider.md)
- [`LocaleSupport`](locale-support.md)
- [`RenderContext`](../rendering/render-context.md)

## See Also

- [Cookbook: Localization](../../cookbook/localization.md)
