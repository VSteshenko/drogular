add_executable(drogular_cli
    main.cpp
)

set_target_properties(drogular_cli PROPERTIES
    OUTPUT_NAME drogular
)

target_compile_features(drogular_cli
    PRIVATE
        cxx_std_20
)

target_compile_definitions(drogular_cli
    PRIVATE
        DROGULAR_CLI_VERSION="${PROJECT_VERSION}"
)
