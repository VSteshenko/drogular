add_executable(mini_store
    main.cpp
)

target_link_libraries(mini_store PRIVATE Drogular::drogular)
target_include_directories(mini_store PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})