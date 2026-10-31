# Drogular 0.24.0

Drogular 0.24 strengthens the application layer built on the 0.23 server-driven UI foundations. The release adds 
file-backed localization and catalog validation, evolves PortalDemo into a more complete application shell, 
and introduces Mini Store as a second reference application covering customer and staff workflows.

## Highlights

### External Localization Resources

`FileTranslationProvider` allows applications to keep translation catalogs in external JSON files instead of embedding 
all translations in application code.

The localization model provides deterministic fallback behavior:

- requested locale;
- configured default locale;
- translation key when no translation is available.

This keeps server rendering as the source of translation truth while making catalogs easier to maintain and deploy.

### Localization Validation Tooling

0.24 adds `drogular-l10n-check`, a standalone command-line validator for translation catalogs.

The tool can compare locale catalogs against a reference locale and report missing or extra keys. Example applications 
integrate catalog validation with CTest/CI so incomplete translation resources fail alongside normal regression tests.

### PortalDemo Application Shell

PortalDemo now uses a more application-oriented frame:

- persistent top header;
- persistent left navigation;
- dedicated replaceable content area;
- Drogular Interactions for content navigation;
- responsive behavior for smaller screens.

The dashboard also presents current project work using the existing project model. First-class task status, priority, 
summaries, and Interaction-based task updates are intentionally deferred to 0.25.

## Mini Store Reference Application

0.24 introduces Mini Store as a second substantial reference application alongside PortalDemo. It uses one shared domain
and service layer with separate Storefront and Staff presentation surfaces.

### Storefront

The customer-facing flow includes:

- home and catalog pages;
- categories, search, filters, sorting, and product details;
- session-backed cart;
- checkout with customer and stock validation;
- order creation and confirmation.

### Staff

The staff surface includes:

- dashboard;
- product listing and editing;
- category listing and editing;
- order filtering and details;
- order workflow from New to Processing to Completed.

Order items preserve product-name and unit-price snapshots so existing orders remain stable when catalog data changes.

### Localization, Interactions, and PWA

Mini Store uses external English and German translation catalogs as an acceptance test for the 0.24 localization APIs. 
Domain content uses localized values with requested-locale → default-locale fallback.

The application also exercises:

- Drogular UI across Storefront and Staff;
- `dg-get` / `dg-post` Interaction flows;
- canonical URLs and browser history for filters and navigation;
- safe return-navigation propagation in application flows;
- light/dark themes and language switching;
- manifest, Service Worker, application-shell caching, and offline fallback.

## Testing and Release Quality

0.24 adds application-level Mini Store regression coverage on top of its domain and support tests. The suite covers 
catalog/product rendering, session cart behavior, checkout validation and order creation, staff product/category surfaces, 
order filtering/details/transitions, fragment responses, localization, canonical navigation, and history behavior.

Translation catalogs are also validated through CTest/CI. Together with the existing framework and example suites, 
this keeps the new reference application tied to the public APIs it is intended to demonstrate.

## Documentation

The English and German root READMEs, demo index, localization reference, cookbook material, and example documentation 
were synchronized with the 0.24 APIs and reference applications.

## Looking Ahead

0.25 will extend localization into client and offline scenarios while preserving the server catalog as the source of truth. 
It will also introduce a first-class Return URL / Navigation Context API and expand PortalDemo task modeling with status,
priority, summaries, and Interaction-based updates.
