add_executable(drogular_cli
    main.cpp
    project_generator.cpp
    template_engine.cpp
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
        DROGULAR_CLI_TEMPLATE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/templates"
        DROGULAR_CLI_INSTALL_TEMPLATE_DIR="${CMAKE_INSTALL_FULL_DATADIR}/drogular/templates"
)

install(
    TARGETS drogular_cli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    DIRECTORY templates/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/drogular/templates
)
