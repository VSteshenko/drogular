# Client Localization

**Namespace:** `drogular` (catalog); browser global `drogularLocalization` (runtime)
**Header:** `<drogular/client_localization_catalog.hpp>`
**Opt-in:** `App::clientLocalization()`

## Purpose

The client catalog exports browser-readable translations from the same file-based source used for server rendering.
Applications own their HTTP manifest/resource endpoints; enabling the runtime does **not** register application translation routes.

## Server contract

```cpp
#include <drogular/client_localization_catalog.hpp>
#include <drogular/file_translation_provider.hpp>

drogular::FileTranslationProvider provider("resources/localization", "en");
drogular::ClientLocalizationCatalog catalog(
    provider.catalog(), provider.defaultLocale(), "/assets/localization");

const auto manifest = catalog.manifest().toJson();
const auto german = catalog.resource("de").toJson();
```

The default resource suffix is `.json`. The fourth constructor argument may override it (including `""` for extensionless 
routes). PortalDemo uses extensionless `/assets/localization/{locale}` because its router reads the named parameter directly.

Manifest example (versions abbreviated):

```json
{
  "defaultLocale": "en",
  "version": "<content-version>",
  "locales": {
    "en": {"url": "/assets/localization/en.json", "version": "<en-version>"},
    "de": {"url": "/assets/localization/de.json", "version": "<de-version>"}
  }
}
```

Each resource contains `locale`, `version` and a flat `translations` object. Changes to any locale change the manifest 
version; per-locale versions change only when that locale's content changes. Missing or invalid default locales and 
invalid resource paths are rejected during catalog construction.

## Browser runtime

Enable the asset at application startup and include the script in your layout:

```cpp
app.clientLocalization();
```

```html
<script src="/__drogular/assets/localization.js" defer></script>
```

After the script loads, configure it with the manifest endpoint you registered:

```js
const l10n = globalThis.drogularLocalization;
l10n.configure({manifestUrl: "/assets/localization/manifest.json"});
await l10n.ready(document.documentElement.lang || "en");
const title = l10n.translate("nav.dashboard");
await l10n.setLocale("de");
```

The runtime also exposes `loadManifest()`, `loadResource(locale)`, `locale()`, `defaultLocale()`, `manifest()` and 
`cacheName()`. `setLocale()` updates `<html lang>` and dispatches `dg:locale-changed` with `event.detail.locale`. 
Listeners should update presentation, **not** call `setLocale()` again from this event (which would create a loop).

Client fallback matches `FileTranslationProvider`: active locale → default locale → original key. Unknown requested 
locales resolve to the default locale. Loading can fail if neither network nor the needed cached resource is available; 
call `ready()` or `setLocale()` and handle rejections before relying on localized UI.

## Offline and cache lifecycle

The runtime stores the manifest and versioned locale responses in Cache Storage namespace `drogular-localization-v1` 
with network → cached-response fallback. On successful manifest refresh, obsolete resource versions are pruned while 
unchanged locale versions remain cached. The application shell Service Worker is separate; if offline page startup is 
required, **precache the runtime script** and preserve the localization namespace when cleaning up shell caches.

In PortalDemo, `service-worker.js` retains caches belonging to other namespaces and precaches
`/__drogular/assets/localization.js`. Browser-cached translations are not interchangeable with Offline Read Models'
locale-aware server-rendered representations.

## Related

- [`FileTranslationProvider`](file-translation-provider.md)
- [Localization cookbook](../../cookbook/localization.md)
- [Offline Read Models](../offline/README.md)
- [`App`](../application/app.md)