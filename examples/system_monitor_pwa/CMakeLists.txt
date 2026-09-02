set(SYSTEM_MONITOR_CORE_SOURCES
    hardware/board_gpio_metadata.cpp
    hardware/i2c_gpio_correlator.cpp
    hardware/linux_hardware_capability_probe.cpp
    hardware/spi_gpio_correlator.cpp
    hardware/uart_gpio_correlator.cpp
    services/gpio_service.cpp
    services/i2c_service.cpp
    services/process_service.cpp
    services/spi_service.cpp
    services/uart_service.cpp
    services/system_monitor.cpp
    system/process_list_parser.cpp
    ui/system_formatters.cpp
    platform/linux_system_metrics_provider.cpp
    platform/raspberry_pi_probe.cpp
    gpio/gpiod_gpio_provider.cpp
    i2c/i2c_tools_provider.cpp
    spi/spidev_spi_provider.cpp
    uart/linux_uart_provider.cpp
    ssh/ssh_system_reader.cpp
)


find_path(SYSTEM_MONITOR_LIBSSH_INCLUDE_DIR
    NAMES libssh/libssh.h
)

find_library(SYSTEM_MONITOR_LIBSSH_LIBRARY
    NAMES ssh libssh
)

set(SYSTEM_MONITOR_HAS_LIBSSH OFF)
if(SYSTEM_MONITOR_LIBSSH_INCLUDE_DIR AND SYSTEM_MONITOR_LIBSSH_LIBRARY)
    set(SYSTEM_MONITOR_HAS_LIBSSH ON)
    list(APPEND SYSTEM_MONITOR_CORE_SOURCES
        ssh/libssh_client.cpp
    )
endif()

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

if(SYSTEM_MONITOR_HAS_LIBSSH)
    target_include_directories(system_monitor_core
        PRIVATE
            ${SYSTEM_MONITOR_LIBSSH_INCLUDE_DIR}
    )

    target_link_libraries(system_monitor_core
        PUBLIC
            ${SYSTEM_MONITOR_LIBSSH_LIBRARY}
    )

    target_compile_definitions(system_monitor_core
        PUBLIC
            SYSTEM_MONITOR_HAS_LIBSSH=1
    )
else()
    target_compile_definitions(system_monitor_core
        PUBLIC
            SYSTEM_MONITOR_HAS_LIBSSH=0
    )
endif()

if((APPLE OR CMAKE_SYSTEM_NAME STREQUAL "Linux") AND DROGULAR_BUILD_EXAMPLES)
    add_executable(system_monitor_pwa
        main.cpp
        pages/dashboard_page.cpp
        pages/board_page.cpp
        actions/gpio_status_action.cpp
        actions/i2c_status_action.cpp
        actions/process_status_action.cpp
        actions/spi_status_action.cpp
        actions/uart_status_action.cpp
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