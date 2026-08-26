set(SYSTEM_MONITOR_CORE_SOURCES
    services/system_monitor.cpp
    ui/system_formatters.cpp
)

if(APPLE)
    list(APPEND SYSTEM_MONITOR_CORE_SOURCES
        platform/macos_system_metrics_provider.cpp
    )
endif()

add_library(system_monitor_core STATIC
    ${SYSTEM_MONITOR_CORE_SOURCES}
)

target_include_directories(system_monitor_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(system_monitor_core
    PUBLIC
        cxx_std_20
)

if(APPLE AND DROGULAR_BUILD_EXAMPLES)
    add_executable(system_monitor_pwa
        main.cpp
        pages/dashboard_page.cpp
    )

    target_link_libraries(system_monitor_pwa
        PRIVATE
            Drogular::drogular
            system_monitor_core
    )

    target_include_directories(system_monitor_pwa
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()