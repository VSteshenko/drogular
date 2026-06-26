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

⸻

## Audit

### Authentication

Current:

Typical page initialization:

```c++
PortalPageSupport::apply(context, "...");
auto currentUser =
    PortalAuthSupport::currentUser(context);
context.set(
    "loginRequired",
    !currentUser.has_value()
);
```

#### Observation

This pattern appears in:

- Dashboard
- Users
- Projects
- Admin

#### Candidate

Possible future APIs:

- AuthenticatedPage
- AdminPage

Status:

Not enough evidence yet.

⸻

### Localization

Current:

Typical page:

```c++
auto translations =
    context.requireService<PortalTranslations>();
const auto locale =
    PortalLocale::fromRenderContext(context);
```

#### Observation

Appears in nearly every page.

Needs more investigation before introducing a localization service.

Status:

Observe.

⸻

### Error Handling

Current:

PortalErrorTranslator

Shared alert partials

Localized validation messages

#### Observation

Current implementation is small and understandable.

No additional abstraction needed.

Status:

Keep as-is.

⸻

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

⸻

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

⸻

### Page Support

Current:

PortalPageSupport::apply(…)

Provides:

- localization
- navigation
- common template variables
- PWA support

#### Observation

Already significantly reduces boilerplate.

May become a framework feature in the future.

Status:

Good candidate.

⸻

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

⸻

## Page Initialization

Current:

PageSupport::apply(...)

resolve services

context.set(...)

render

#### Observation

Every page follows the same initialization pattern.

Status:

Observe

⸻

## Candidate APIs

The following ideas are intentionally postponed until more evidence is collected.

- AuthenticatedPage
- AdminPage
- CrudPage
- CrudAction
- LocalizationService
- AlertComponent
- FormBuilder
- CRUD helpers

None of these should be implemented before they naturally emerge from multiple applications.

⸻

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
