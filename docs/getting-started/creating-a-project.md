# Creating a Project with the Drogular CLI

The Drogular CLI creates ready-to-build applications from embedded project templates. No template files need to be installed separately.

## Create a minimal project

```bash
drogular new hello_drogular
cd hello_drogular
cmake -S . -B build
cmake --build build
./build/hello_drogular
```

Then open `http://localhost:8080/`.

`minimal` is the default template. It keeps the generated application deliberately small while showing the recommended Page, Component, template, public-asset, and startup structure.

## Choose the output path

The project argument can include a relative or absolute destination path:

```bash
drogular new examples/hello_drogular
drogular new ../generated/hello_drogular
drogular new /tmp/hello_drogular
```

The final path component becomes the project name. For `examples/admin/My-App`, the project name is `My-App`; the generated C++ namespace is normalized to `My_App`.

The destination must not already exist. This prevents the generator from silently overwriting an existing project.

## List project templates

The installed CLI can describe the starters embedded in that build:

```bash
drogular templates
```

| Template | Purpose |
|----------|---------|
| `minimal` | Smallest recommended Drogular application; best for learning the core project structure. |
| `pwa` | Ready-to-run installable PWA with an application shell, manifest, service worker, offline fallback, icons, and responsive starter UI. |

## Select a template

```bash
drogular new MyPWA --template pwa
drogular new apps/customer-portal --template=pwa
```

An unknown template is rejected. Run `drogular templates` to see the identifiers supported by the installed CLI.

## What the minimal starter creates

```text
hello_drogular/
├── CMakeLists.txt
├── README.md
├── public/
├── src/
│   ├── components/
│   │   └── home_component.hpp
│   ├── home_page.hpp
│   └── main.cpp
└── templates/
    ├── components/
    │   └── home.html
    └── home.html
```

The generated source contains short `Tip:` comments at useful extension points without turning the starter into a tutorial.

## What the PWA starter adds

The `pwa` template keeps the same Drogular structure and adds the pieces needed for an installable application:

- PWA Page metadata;
- reusable application shell;
- `manifest.webmanifest`;
- root-scoped service worker;
- offline navigation fallback;
- starter icons and favicon;
- responsive starter UI.

After building and running the project on localhost, the browser can register the service worker and install the application without additional PWA setup.

## Drogular version used by generated projects

Generated `CMakeLists.txt` files are pinned to the Drogular Git tag corresponding to the CLI build:

```cmake
FetchContent_Declare(
    drogular
    GIT_REPOSITORY https://github.com/VSteshenko/drogular.git
    GIT_TAG v0.21.0
)
```

The concrete tag above is only an example. A released CLI writes its own matching release tag, keeping generated projects reproducible instead of silently following future changes on `main`.

When developing Drogular itself and testing a generated project against the local checkout, override FetchContent at configure time instead of changing the generated template:

```bash
cmake -S . -B build \
  -DFETCHCONTENT_SOURCE_DIR_DROGULAR=/absolute/path/to/drogular
```

This keeps release pinning intact while letting framework development test the starter against the current source tree.

## CLI reference

```text
drogular new <path/to/project> [--template <id>]
drogular templates
drogular --help
drogular --version
```

`drogular new` fails rather than leaving a partially generated project when generation cannot complete successfully.

## Next steps

Continue with [Your First Drogular Application](your-first-drogular-application.md) to understand how Pages, Components, templates, dependency injection, and application startup fit together.

For a new installable web application, the generated `pwa` starter is intended to be a practical starting point rather than only a demonstration.
