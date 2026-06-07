add_executable(todo_pwa
    main.cpp
)

target_link_libraries(todo_pwa
    PRIVATE
        Drogular::drogular
)
