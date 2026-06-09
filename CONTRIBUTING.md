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