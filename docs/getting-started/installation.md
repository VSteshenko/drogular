# Installation

This guide explains how to add Drogular to an existing CMake project.

Currently, the recommended installation method is **CMake FetchContent**.

---

# Requirements

Before installing Drogular, make sure you have:

- A C++20 compatible compiler
- CMake 3.24 or newer
- Git
- Drogon and its dependencies supported by your platform

---

# Add Drogular

Add the following to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    drogular
    GIT_REPOSITORY https://github.com/VSteshenko/drogular.git
    GIT_TAG main
)

FetchContent_MakeAvailable(drogular)
```

---

# Link your application

Link Drogular to your executable:

```cmake
target_link_libraries(MyApplication
    PRIVATE
        drogular
)
```

---

# Verify the installation

Build your project:

```bash
cmake -B build
cmake --build build
```

If the build completes successfully, Drogular has been installed correctly.

---

# What's next?

Continue with:

- First Application
