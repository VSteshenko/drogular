add_executable(developer_tools_example
    main.cpp
)

target_link_libraries(developer_tools_example
    PRIVATE
        Drogular::drogular
)

target_include_directories(developer_tools_example
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)