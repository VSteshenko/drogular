cmake_minimum_required(VERSION 3.20)

project(Drogular
    VERSION 0.1.0
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
    src/router.cpp
    src/graphql.cpp
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
    enable_testing()
    add_subdirectory(tests)
endif()
