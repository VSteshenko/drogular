add_executable(drogular_tests
    app_tests.cpp
    component_tests.cpp
    graphql_result_tests.cpp
    graphql_tests.cpp
    graphql_client_tests.cpp
    render_context_tests.cpp
    services_tests.cpp
    testing_tests.cpp
    todo_page_tests.cpp
    ../examples/todo_pwa/todo_page.cpp
)

target_link_libraries(drogular_tests
    PRIVATE
    Drogular::drogular
    GTest::gtest_main
)

target_include_directories(drogular_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/examples/todo_pwa
)

include(GoogleTest)

gtest_discover_tests(drogular_tests)