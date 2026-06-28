# Developer Experience Audit

This document tracks repetitive patterns discovered while building real Drogular applications.

The purpose is not to invent new APIs, but to identify opportunities where the framework can reduce boilerplate without sacrificing clarity.

## Design Principles

Every new abstraction must satisfy all of the following conditions:

- It has been implemented at least twice in real applications.
- It removes repetitive code.
- It makes application code easier to understand.
- It does not hide important application behavior.

When in doubt, keep the existing API.

---

## Status Legend

| Status | Meaning |
|--------|---------|
| Observe | Repetition has been noticed, but more evidence is needed. |
| Candidate | The pattern appears in multiple places and may justify a helper. |
| Prototype | Implemented as a helper or application-level abstraction. |
| Framework | Proven by real usage and moved into Drogular core. |
| Keep as-is | The current implementation is simple enough and does not need abstraction. |

---

## Implemented in 0.20

### PageSupport

Current framework API:

```c++
drogular::PageSupport::apply(
    context,
    "Dashboard"
);
```

Provides:

- `pageTitle`
- `currentPath`
- PWA metadata through `PwaPageSupport`
- common page-level setup

#### Result

Common page setup was removed from TodoPWA and Portal Demo pages.

Status:

Framework.

---

### AuthSupport

Current framework API:

```c++
drogular::AuthSupport::sessionValue(
    context,
    "username"
);

drogular::AuthSupport::hasSessionValue(
    context,
    "role",
    "admin"
);
```

#### Result

Low-level session lookup code was removed from `auth_sample` and `portal_demo` helpers.

Status:

Framework.

---

### PageAuthSupport

Current framework API:

```c++
if (!drogular::PageAuthSupport::requireAuthentication(context)) {
    return;
}

if (!drogular::PageAuthSupport::requireSessionValue(
        context,
        "role",
        "admin")) {
    return;
}
```

Provides common render context flags:

- `loginRequired`
- `accessDenied`

#### Result

Repeated authentication checks in Dashboard, Users, Projects and Admin pages were reduced.

Status:

Framework.

---

### LocaleSupport

Current framework API:

```c++
const auto locale =
    drogular::LocaleSupport::current(context);
```

#### Result

Portal-specific locale detection was replaced with a small core helper.

Status:

Framework.

---

### TranslationProvider and TranslationSupport

Current framework API:

```c++
class PortalTranslations
    : public drogular::TranslationProvider
{
public:
    std::string translate(
        const std::string& locale,
        const std::string& key
    ) const override;
};
```

```c++
drogular::TranslationSupport::translate(
    context,
    "dashboard.title"
);
```

#### Result

Pages no longer need to know how locale and translation provider are connected.

Status:

Framework.

---

### RenderContext::translate

Current framework API:

```c++
context.translate("dashboard.title");
```

#### Result

Translation became a natural render-context operation.

This reduced repeated code such as:

```c++
const auto locale =
    drogular::LocaleSupport::current(context);

auto translations =
    context.requireService<drogular::TranslationProvider>();
```

Status:

Framework.

---

## Audit

### Authentication

Current:

```c++
if (!drogular::PageAuthSupport::requireAuthentication(context)) {
    return;
}
```

Admin checks:

```c++
if (!drogular::PageAuthSupport::requireSessionValue(
        context,
        "role",
        "admin")) {
    return;
}
```

#### Observation

The first small helper layer is now in core.

Higher-level abstractions may still appear later.

#### Candidate

Possible future APIs:

- AuthenticatedPage
- AdminPage

Status:

Observe.

---

### Localization

Current:

```c++
context.translate("app.title");
```

#### Observation

The first localization layer is now in core:

- `LocaleSupport`
- `TranslationProvider`
- `TranslationSupport`
- `RenderContext::translate()`

Future growth may include:

- file-based providers
- cached providers
- fallback locale chains
- machine translation providers
- formatting helpers

Status:

Framework foundation implemented. Observe next layer.

---

### Translation Context Setup

Current:

```c++
context.set(
    "navDashboard",
    context.translate("nav.dashboard")
);

context.set(
    "navUsers",
    context.translate("nav.users")
);
```

#### Observation

After adding `RenderContext::translate()`, a new pattern appeared: translate a key and immediately place it into the render context.

This is not yet enough evidence for a new API.

Status:

Observe.

---

### Error Handling

Current:

- PortalErrorTranslator
- Shared alert partials
- Localized validation messages
- `context.translate(...)`

#### Observation

Current implementation is small and understandable.

No additional abstraction needed.

Status:

Keep as-is.

---

### CRUD Pages

Current flow:

```text
Model
↓
Repository
↓
Page
↓
Action
↓
Template
↓
Translations
↓
Validation
```

#### Observation

Users and Projects now follow almost identical architecture.

Need additional examples before introducing CRUD helpers.

Status:

Observe.

---

### Repository Pattern

Current repositories expose:

- all()
- create()
- exists()

#### Observation

Very small API.

No unnecessary abstraction.

Status:

Keep as-is.

---

### Page Initialization

Current:

```c++
drogular::PageSupport::apply(...);

resolve services

context.set(...)

render
```

#### Observation

Every page follows the same initialization pattern, but the current helpers already reduced the largest repeated parts.

Status:

Observe.

---

### Actions

Typical action:

- validate
- resolve services
- execute business logic
- redirect

#### Observation

Still simple.

Needs more real applications before abstraction.

Status:

Observe.

---

## Candidate APIs

The following ideas are intentionally postponed until more evidence is collected.

- AuthenticatedPage
- AdminPage
- CrudPage
- CrudAction
- AlertComponent
- FormBuilder
- CRUD helpers
- Translation context binding helpers
- File-based TranslationProvider
- Cached TranslationProvider

None of these should be implemented before they naturally emerge from multiple applications.

---

## Rule

Framework APIs should evolve in this order:

```text
Real Application
↓
Repeated Pattern
↓
Small Helper
↓
Public Framework API
```

Never in the opposite direction.
