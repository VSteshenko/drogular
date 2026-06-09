cmake_minimum_required(VERSION 3.24)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

project(Drogular
    VERSION 0.6.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(DROGULAR_BUILD_EXAMPLES "Build Drogular examples" ON)
option(DROGULAR_BUILD_TESTS "Build Drogular tests" ON)

find_package(Drogon REQUIRED)

add_library(drogular
    src/app.cpp
    src/component.cpp
    src/page.cpp
    src/router.cpp
    src/graphql.cpp
    src/graphql_client.cpp
    src/services.cpp
    src/template_engine.cpp
)

add_library(Drogular::drogular ALIAS drogular)

target_include_directories(drogular
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(drogular
    PUBLIC
        Drogon::Drogon
)

target_compile_features(drogular
    PUBLIC
        cxx_std_20
)

if(DROGULAR_BUILD_EXAMPLES)
    add_subdirectory(examples/todo_pwa)
endif()

if(DROGULAR_BUILD_TESTS)

    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_subdirectory(tests)

endif()
