add_executable(drogular_tests
    app_tests.cpp
    component_registry_tests.cpp
    component_renderer_tests.cpp
    component_tests.cpp
    dependency_graph_tests.cpp
    graphql_client_tests.cpp
    graphql_result_tests.cpp
    graphql_tests.cpp
    inject_tests.cpp
    render_context_tests.cpp
    services_tests.cpp
    template_component_tests.cpp
    template_engine_tests.cpp
    template_page_tests.cpp
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