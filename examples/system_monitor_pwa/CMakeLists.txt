if(APPLE)
    add_library(system_monitor_core STATIC
        platform/macos_system_metrics_provider.cpp
    )

    target_include_directories(system_monitor_core
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_compile_features(system_monitor_core
        PUBLIC
            cxx_std_20
    )
else()
    add_library(system_monitor_core INTERFACE)

    target_include_directories(system_monitor_core
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_compile_features(system_monitor_core
        INTERFACE
            cxx_std_20
    )
endif()