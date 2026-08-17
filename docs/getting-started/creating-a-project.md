# Creating a Project with the Drogular CLI

The Drogular CLI can create a minimal application with the recommended project structure already in place.

## Create the project

```bash
drogular new hello_drogular
cd hello_drogular
```

The generated project contains:

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

The project is intentionally small. It demonstrates the same basic organization used throughout the Getting Started guide: application startup in `main.cpp`, Pages in application code, reusable Components, and external templates.

## Build and run

```bash
cmake -S . -B build
cmake --build build
./build/hello_drogular
```

Then open:

```text
http://localhost:8080/
```

## Drogular version used by generated projects

A released Drogular CLI generates a project pinned to the corresponding Drogular Git tag. For example, the `0.21.0` CLI writes:

```cmake
FetchContent_Declare(
    drogular
    GIT_REPOSITORY https://github.com/VSteshenko/drogular.git
    GIT_TAG v0.21.0
)
```

This keeps newly generated applications reproducible instead of silently following future changes on `main`.

When building the CLI itself from source, the Git ref can be overridden explicitly:

```bash
cmake -S . -B build -DDROGULAR_CLI_GIT_REF=main
```

A branch name, tag, or commit hash can be used as the value.

## Next steps

Continue with [Your First Drogular Application](your-first-drogular-application.md) to understand how the generated Page, Component, templates, dependency injection, and startup sequence fit together.
