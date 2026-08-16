# Installation

This guide creates the smallest useful CMake project that links Drogular and is ready for the next tutorial.

The recommended source-integration method is **CMake FetchContent**.

---

# Requirements

Before configuring the project, make sure you have:

- a C++20-compatible compiler
- CMake 3.24 or newer
- Git
- Drogon and its platform dependencies installed so `find_package(Drogon REQUIRED)` can locate them

Drogular links Drogon transitively, but Drogon itself must be discoverable by CMake when Drogular is configured.

---

# Create the Project

Start with this layout:

```text
hello_drogular/
├── CMakeLists.txt
└── src/
    └── main.cpp
```

Create `CMakeLists.txt` in the project root:

```cmake
cmake_minimum_required(VERSION 3.24)

project(HelloDrogular LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# A consuming application normally does not need Drogular's own
# examples and tests.
set(DROGULAR_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(DROGULAR_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    drogular
    GIT_REPOSITORY https://github.com/VSteshenko/drogular.git
    GIT_TAG main
)

FetchContent_MakeAvailable(drogular)

add_executable(hello_drogular
    src/main.cpp
)

target_link_libraries(hello_drogular
    PRIVATE
        Drogular::drogular
)
```

The namespaced target `Drogular::drogular` is the recommended target for application code.

---

# Add a Minimal `main.cpp`

Create `src/main.cpp`:

```cpp
#include <drogular/app.hpp>

int main()
{
    drogular::App app;

    app.run(8080);

    return 0;
}
```

At this point the application has no routes yet. The goal is only to verify that your toolchain, Drogon, Drogular, and the application target are wired together correctly.

---

# Configure and Build

From the project root:

```bash
cmake -S . -B build
cmake --build build
```

Run the executable:

```bash
./build/hello_drogular
```

The process should start and listen on port `8080`.

> Depending on your generator or platform, the executable may be placed in a configuration-specific subdirectory such as `build/Debug/`.

---

# If CMake Cannot Find Drogon

An error mentioning `DrogonConfig.cmake` means Drogon is either not installed or is installed outside CMake's search path.

Install Drogon for your platform or point CMake at its package directory, for example through `CMAKE_PREFIX_PATH` or `Drogon_DIR`, then configure the project again.

---

# What's Next?

Continue with [Your First Drogular Application](your-first-drogular-application.md). It builds on this exact project and adds a Page, Component, Service, template files, dependency injection, and route registration.
