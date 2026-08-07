# TranslationProvider

**Namespace:** `drogular`  
**Header:** `<drogular/translation_provider.hpp>`  
**Kind:** Abstract class

## Purpose

`TranslationProvider` defines the application-specific translation lookup contract.

Drogular supplies the requested locale and translation key. The application decides where translations are stored and what fallback policy to apply.

---

## Synopsis

```cpp
class TranslationProvider {
public:
    virtual ~TranslationProvider() = default;

    virtual std::string translate(
        const std::string& locale,
        const std::string& key
    ) const = 0;
};
```

---

## Public API

### `translate()`

```cpp
virtual std::string translate(
    const std::string& locale,
    const std::string& key
) const = 0;
```

Returns the text associated with `key` for `locale`.

The interface does not define behavior for:

- unsupported locales;
- missing translation keys;
- fallback languages;
- storage failures.

Those policies belong to the concrete implementation.

---

## Registration

Register an implementation through dependency injection:

```cpp
app.services().addFactory<drogular::TranslationProvider>(
    drogular::ServiceLifetime::Singleton,
    [] {
        return std::make_shared<PortalTranslations>();
    }
);
```

`TranslationSupport` resolves the provider from the current `RenderContext`.

---

## Behavior

`TranslationProvider` itself has no built-in storage or fallback behavior.

When no provider is registered, `TranslationSupport` returns the original translation key instead of throwing.

---

## Example

```cpp
class PortalTranslations final
    : public drogular::TranslationProvider
{
public:
    std::string translate(
        const std::string& locale,
        const std::string& key
    ) const override
    {
        const auto localeIt = values_.find(locale);

        if (localeIt == values_.end()) {
            return fallback(key);
        }

        const auto valueIt =
            localeIt->second.find(key);

        if (valueIt == localeIt->second.end()) {
            return fallback(key);
        }

        return valueIt->second;
    }
};
```

PortalDemo uses this pattern in `PortalTranslations`.

---

## Related Types

- [`TranslationSupport`](translation-support.md)
- [`LocaleSupport`](locale-support.md)
- [`RenderContext`](../rendering/render-context.md)

## See Also

- [Cookbook: Localization](../../cookbook/localization.md)
