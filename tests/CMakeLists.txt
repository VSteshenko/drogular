add_executable(drogular_tests
        app_tests.cpp
        component_tests.cpp
        graphql_result_tests.cpp
        graphql_tests.cpp
        render_context_tests.cpp
)

target_link_libraries(drogular_tests
        PRIVATE
        Drogular::drogular
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(drogular_tests)