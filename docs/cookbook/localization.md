# Localization

## Problem

**Need to support multiple languages?**

This guide shows how to separate user-visible text from application logic using Drogular's localization infrastructure.

---

## Recommended Solution

Implement a `TranslationProvider` and resolve translated values through `RenderContext`.

This keeps pages and components independent from any particular language while allowing the application to choose how translations are stored.

---

## How It Works

### Translation Provider

`TranslationProvider` defines a simple interface for resolving localized text.

Applications are free to implement the interface using any storage mechanism. PortalDemo provides a concrete implementation named `PortalTranslations`.

```cpp
class TranslationProvider
{
public:
    virtual ~TranslationProvider() = default;

    virtual std::string translate(
        const std::string& locale,
        const std::string& key
    ) const = 0;
};
```

---

### Registering the Provider

Register the translation provider through dependency injection.

```cpp
app.services().addFactory<drogular::TranslationProvider>(
    drogular::ServiceLifetime::Singleton,
    [] {
        return std::make_shared<PortalTranslations>();
    }
);
```

Using a singleton ensures that the application shares a single translation provider.

---

### Translation Keys

Translation keys should reflect the application domain rather than the organization of translation files.

For example:

```text
nav.dashboard
users.title
users.search.label
departments.edit.title
common.previous
```

A consistent naming scheme keeps translations stable as the application evolves.

---

## Example

Translate user-visible text through `RenderContext`.

```cpp
const auto title =
    context.translate("users.title");
```

`RenderContext` resolves the current locale and delegates the lookup to the registered translation provider.

```text
RenderContext
      │
      ▼
Current Locale
      │
      ▼
TranslationProvider
      │
      ▼
Localized Text
```

Translated values are then passed to the template.

```cpp
context.set(
    "pageTitle",
    context.translate("users.title")
);
```

When several values must be translated, register them together.

```cpp
context.setTranslations({
    {"pageTitle", "users.title"},
    {"navUsers", "nav.users"},
    {"navDepartments", "nav.departments"},
    {"commonPrevious", "common.previous"},
    {"commonNext", "common.next"}
});
```

This keeps page initialization concise while making every translated value explicit.

---

### Fallback Behavior

Fallback behavior is defined by the translation provider implementation.

`PortalTranslations` first searches the requested locale.

If the locale or translation key cannot be found, it falls back to the default language.

Applications may implement a different fallback strategy while using the same `TranslationProvider` interface.

---

## Best Practices

- Keep translation keys stable.
- Organize translation keys by application domain.
- Keep user-visible text outside pages and components.
- Register translations through `TranslationProvider`.
- Keep locale selection separate from translation lookup.
- Let the translation provider define storage and fallback behavior.

---

## See Also

### Getting Started *(coming soon)*

- Localization

### API Reference

- [TranslationProvider](../reference/localization/translation-provider.md)
- [TranslationSupport](../reference/localization/translation-support.md)
- [LocaleSupport](../reference/localization/locale-support.md)
- RenderContext *(coming soon)*
