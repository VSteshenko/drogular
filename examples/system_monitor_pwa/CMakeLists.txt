set(SYSTEM_MONITOR_CORE_SOURCES
    services/system_monitor.cpp
    ui/system_formatters.cpp
    platform/linux_system_metrics_provider.cpp
)

if(APPLE)
    list(APPEND SYSTEM_MONITOR_CORE_SOURCES
        platform/macos_system_metrics_provider.cpp
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND SYSTEM_MONITOR_CORE_SOURCES
        platform/local_linux_system_reader.cpp
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

if((APPLE OR CMAKE_SYSTEM_NAME STREQUAL "Linux") AND DROGULAR_BUILD_EXAMPLES)
    add_executable(system_monitor_pwa
        main.cpp
        pages/dashboard_page.cpp
        actions/system_status_action.cpp
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