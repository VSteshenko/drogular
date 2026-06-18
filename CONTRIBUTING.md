## `CONTRIBUTING.md`

# Contributing to Drogular

Thank you for your interest in Drogular.

Drogular is an experimental C++ framework built on top of Drogon. The project is still small, so contributions should focus on clarity, tests, and simple design.

## Development requirements

You need:

- CMake 3.22 or newer
- C++20 compiler
- Drogon
- Git
- GoogleTest is fetched automatically by CMake

## Build locally

```bash
cmake -S . -B build \
  -DDROGULAR_BUILD_TESTS=ON \
  -DDROGULAR_BUILD_EXAMPLES=ON

cmake --build build
```

## Code Style

- C++20
- English comments only
- One feature per pull request
- Tests required for new functionality
- Public APIs must be documented

## Testing Guidelines

Test-only classes, fixtures, stores, and component tags should use unique, area-prefixed names.

Recommended prefixes:

- `Core` for framework tests
- `TodoPwa` for TodoPWA example tests
- `AuthSample` for authentication sample tests

Examples:

- `CoreLifecycleParentComponent`
- `CoreStorePatternTodoStore`
- `TodoPwaStoreTests`
- `AuthSampleLoginActionTests`

Avoid generic test-only names such as:

- `TodoStore`
- `TodoItemComponent`
- `CounterComponent`
- `ParentComponent`
- `ChildComponent`

This avoids ODR conflicts when tests are linked into a single test binary.