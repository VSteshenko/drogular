add_executable(portal_demo
    main.cpp
)

target_link_libraries(portal_demo
    PRIVATE
        Drogular::drogular
)

target_include_directories(portal_demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)