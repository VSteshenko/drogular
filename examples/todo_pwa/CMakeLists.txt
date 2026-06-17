add_executable(todo_pwa
    main.cpp
    todo_page.cpp
)

target_link_libraries(todo_pwa
    PRIVATE
        Drogular::drogular
)

target_include_directories(todo_pwa
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)