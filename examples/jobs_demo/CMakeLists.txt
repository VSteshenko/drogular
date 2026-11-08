add_executable(
    jobs_demo main.cpp
)
target_link_libraries(
    jobs_demo
    PRIVATE Drogular::drogular
)
target_include_directories(
    jobs_demo
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)