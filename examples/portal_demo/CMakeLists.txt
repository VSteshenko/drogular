add_executable(portal_demo
    main.cpp
    providers/graphql/documents/project_queries.cpp
    providers/graphql/documents/project_mutations.cpp
    providers/graphql/documents/user_queries.cpp
    providers/graphql/documents/user_mutations.cpp
    providers/graphql/documents/project_type_queries.cpp
)

target_link_libraries(portal_demo
    PRIVATE
        Drogular::drogular
)

target_include_directories(portal_demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)