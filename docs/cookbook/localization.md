# Localization

Applications should not embed user-facing text inside components.

Instead, user-visible text is provided through the `TranslationProvider` interface. This keeps pages and components independent of any particular language while allowing each application to choose how translations are stored.

---

# Translation Provider

`TranslationProvider` defines a small interface for resolving localized text.

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

Drogular does not prescribe where translations come from.

An application may use in-memory data, resource files, a database or another source behind the same interface.

PortalDemo includes a concrete implementation named `PortalTranslations`.

```cpp
class PortalTranslations
    : public drogular::TranslationProvider
{
public:
    std::string translate(
        const std::string& locale,
        const std::string& key
    ) const override
    {
        return get(locale, key);
    }
};
```

---

# Registering the Provider

The translation provider is registered through dependency injection.

PortalDemo registers `PortalTranslations` as a singleton:

```cpp
app.services().addFactory<drogular::TranslationProvider>(
    drogular::ServiceLifetime::Singleton,
    [] {
        return std::make_shared<PortalTranslations>();
    }
);
```

The application therefore uses one translation source for every request.

---

# Translation Keys

Translation keys should reflect the structure of the application rather than the organization of translation files.

For example:

```text
nav.dashboard
users.title
users.search.label
departments.edit.title
common.previous
```

PortalDemo stores localized values by locale and key:

```cpp
std::unordered_map<
    std::string,
    std::unordered_map<std::string, std::string>
> values_{
    {
        "en",
        {
            {"nav.dashboard", "Dashboard"},
            {"users.title", "Users"},
            {"departments.edit.title", "Edit Department"},
            {"common.previous", "Previous"}
        }
    },
    {
        "de",
        {
            {"nav.dashboard", "Übersicht"},
            {"users.title", "Benutzer"},
            {"departments.edit.title", "Abteilung bearbeiten"},
            {"common.previous", "Zurück"}
        }
    }
};
```

A consistent naming scheme makes translations easier to understand and extend.

---

# Translating Values

Pages translate text through `RenderContext`.

```cpp
const auto pageTitle =
    context.translate("departments.details.title");
```

The context resolves the current locale and delegates the lookup to the registered `TranslationProvider`.

```text
RenderContext
      │
      ▼
Current locale
      │
      ▼
TranslationProvider
      │
      ▼
Localized text
```

Translated values can then be added to the template context.

```cpp
context.set(
    "departmentStatus",
    context.translate(
        department->isActive
            ? "departments.active"
            : "departments.inactive"
    )
);
```

The page decides which translation key is required.

The provider decides which text that key represents.

---

# Setting Multiple Values

When several template values must be translated, `RenderContext` can resolve them together.

```cpp
context.setTranslations({
    {"appTitle", "app.title"},
    {"navDashboard", "nav.dashboard"},
    {"navUsers", "nav.users"},
    {"navDepartments", "nav.departments"},
    {"commonPrevious", "common.previous"},
    {"commonNext", "common.next"}
});
```

Each pair maps a template value name to a translation key.

This keeps repeated translation setup concise without moving translation responsibility into the template.

---

# Current Locale

Locale selection and translation lookup are separate responsibilities.

`LocaleSupport` resolves the current locale from the request:

```cpp
const auto locale =
    drogular::LocaleSupport::current(context);
```

PortalDemo stores the selected language in the `lang` cookie.

```cpp
return drogular::ActionResult::redirect(
    safeRedirect
).cookie(
    "lang",
    normalized
);
```

`TranslationProvider` does not decide which locale is active.

It only resolves a key for the locale it receives.

---

# Fallback Behavior

Fallback behavior belongs to the translation provider implementation rather than the framework itself.

`PortalTranslations` first searches the requested locale:

```cpp
const auto localeFound =
    values_.find(locale);

if (localeFound == values_.end()) {
    return fallback(key);
}
```

If the locale or key is unavailable, it falls back to English.

```cpp
std::string fallback(
    const std::string& key
) const
{
    const auto english =
        values_.find("en");

    if (english == values_.end()) {
        return key;
    }

    const auto found =
        english->second.find(key);

    if (found == english->second.end()) {
        return key;
    }

    return found->second;
}
```

Other applications may use a different fallback strategy while implementing the same `TranslationProvider` interface.

If no provider is registered, Drogular returns the translation key itself.

---

# Component Responsibilities

Pages and components should request translated values instead of embedding user-visible strings directly.

```cpp
context.set(
    "pageTitle",
    context.translate("users.title")
);
```

Avoid:

```cpp
context.set(
    "pageTitle",
    "Users"
);
```

Keeping translated content outside pages and components separates presentation logic from language-specific text.

---

# Best Practices

- Keep translation keys stable.
- Organize keys by application domain.
- Keep user-visible text outside pages and components.
- Register translation implementations through `TranslationProvider`.
- Keep locale selection separate from translation lookup.
- Let the provider implementation define its storage and fallback policy.