add_executable(auth_sample
    main.cpp
    auth_service.cpp
)

target_link_libraries(auth_sample
    PRIVATE
        drogular
)