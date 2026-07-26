add_executable(portal_demo
    main.cpp
    features/projects/graphql/documents/project_queries.cpp
    features/projects/graphql/documents/project_mutations.cpp
    features/users/graphql/documents/user_queries.cpp
    features/users/graphql/documents/user_mutations.cpp
    features/departments/graphql/documents/department_queries.cpp
    features/departments/graphql/documents/department_mutations.cpp
    features/department_members/graphql/documents/department_member_queries.cpp
    features/department_members/graphql/documents/department_member_mutations.cpp
    features/project_types/graphql/documents/project_type_queries.cpp
    features/project_types/graphql/documents/project_type_mutations.cpp
    features/roles/graphql/documents/role_queries.cpp
    features/roles/graphql/documents/role_mutations.cpp
    data/portal_dataset.cpp
)

target_link_libraries(portal_demo
    PRIVATE
        Drogular::drogular
)

target_include_directories(portal_demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)