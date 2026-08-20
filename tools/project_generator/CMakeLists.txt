include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_project_templates.cmake)

drogular_embed_project_templates(
    ${CMAKE_CURRENT_SOURCE_DIR}/templates
    ${CMAKE_CURRENT_BINARY_DIR}/embedded_template_data.cpp
)

add_library(drogular_project_generator
    embedded_template_source.cpp
    file_renderer.cpp
    project_generator.cpp
    template_registry.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/embedded_template_data.cpp
)

add_library(Drogular::project_generator ALIAS drogular_project_generator)

target_include_directories(drogular_project_generator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(drogular_project_generator
    PUBLIC
        cxx_std_20
)