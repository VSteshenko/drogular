add_library(system_monitor_core INTERFACE)

target_include_directories(system_monitor_core
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(system_monitor_core
    INTERFACE
        cxx_std_20
)