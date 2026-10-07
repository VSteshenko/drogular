add_library(drogular_localization_check_support
    localization_check.cpp
)

target_include_directories(drogular_localization_check_support
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(drogular_localization_check_support PUBLIC cxx_std_20)
target_link_libraries(drogular_localization_check_support PUBLIC Drogular::drogular)

if(DROGULAR_BUILD_TOOLS)
    add_executable(drogular_localization_check main.cpp)
    set_target_properties(drogular_localization_check PROPERTIES OUTPUT_NAME drogular-l10n-check)
    target_compile_features(drogular_localization_check PRIVATE cxx_std_20)
    target_link_libraries(drogular_localization_check PRIVATE drogular_localization_check_support)
    install(TARGETS drogular_localization_check RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()