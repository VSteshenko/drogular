# drogular-l10n-check

`drogular-l10n-check` validates a directory of locale JSON files using `FileTranslationProvider` and `TranslationCatalogValidator`. 
It is intended for local development, automated tests, and CI.

The executable is built when `DROGULAR_BUILD_TOOLS=ON`. Test builds also build it so CTest can use the same validation path.

## Usage

```bash
drogular-l10n-check <directory> [--reference <locale>]
```

The reference locale defaults to `en`. The short option `-r` and `--reference=<locale>` form are also accepted.

For example:

```bash
drogular-l10n-check resources/localization --reference en
```

A complete catalog prints:

```text
Localization catalog is valid (reference: en).
```

Missing and extra keys are reported per locale:

```text
Localization catalog validation failed (reference: en).
de:
  missing: projects.search
  extra:   projects.legacyTitle
```

## Exit codes

| Code | Meaning |
|---:|---|
| `0` | Catalog is valid, or help was requested. |
| `1` | Validation found missing or extra keys. |
| `2` | Arguments are invalid or the catalog could not be loaded. |

The distinction between validation failure and usage/load failure makes the command suitable for CI scripts.

## CTest integration

Drogular's own test suite uses the installed logic directly through the built executable. PortalDemo has a CTest gate 
that validates `examples/portal_demo/resources/localization` against `en`, so an incomplete example catalog fails the 
normal test pipeline.

Applications can use the same pattern with CMake `add_test()` or invoke the executable from their CI system.

## Scope

The command compares locale catalogs against one reference locale. It does not scan application source code or templates 
and therefore does not report whether a translation key is actually used by the application.

## See also

- [`TranslationCatalogValidator`](translation-catalog-validator.md)
- [`FileTranslationProvider`](file-translation-provider.md)