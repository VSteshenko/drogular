#include "actions/gpio_status_action.hpp"
#include "actions/i2c_status_action.hpp"
#include "actions/language_action.hpp"
#include "actions/process_status_action.hpp"
#include "actions/spi_status_action.hpp"
#include "actions/uart_status_action.hpp"
#include "actions/system_status_action.hpp"
#include "configuration/runtime_options.hpp"
#include "gpio/gpiod_gpio_provider.hpp"
#include "hardware/linux_hardware_capability_probe.hpp"
#include "i2c/i2c_tools_provider.hpp"
#include "localization/system_monitor_translations.hpp"
#include "spi/spidev_spi_provider.hpp"
#include "uart/linux_uart_provider.hpp"
#include "pages/dashboard_page.hpp"
#include "pages/board_page.hpp"
#include "pages/offline_page.hpp"
#include "platform/linux_system_metrics_provider.hpp"
#if defined(__APPLE__)
#include "platform/macos_system_metrics_provider.hpp"
#elif defined(__linux__)
#include "platform/local_linux_system_reader.hpp"
#else
#error "system_monitor_pwa currently supports macOS and Linux"
#endif
#if SYSTEM_MONITOR_HAS_LIBSSH
#include "ssh/libssh_client.hpp"
#include "ssh/ssh_system_reader.hpp"
#endif
#include "services/gpio_service.hpp"
#include "services/i2c_service.hpp"
#include "services/process_service.hpp"
#include "services/spi_service.hpp"
#include "services/uart_service.hpp"
#include "services/system_monitor.hpp"
#include "system/monitor_target.hpp"
#include "system/system_metrics_provider.hpp"

#include <drogular/app.hpp>
#include <drogular/static_file_cache_profile.hpp>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

#if SYSTEM_MONITOR_HAS_LIBSSH
[[nodiscard]] std::shared_ptr<system_monitor::SshSystemReader> makeSshReader(
    const system_monitor::SshTargetOptions& options) {
    auto client = std::make_shared<system_monitor::LibsshClient>();
    client->connect(options);
    return std::make_shared<system_monitor::SshSystemReader>(client);
}
#endif

[[nodiscard]] std::shared_ptr<system_monitor::SystemMetricsProvider> makeSshProvider(
    const system_monitor::SshTargetOptions& options) {
#if SYSTEM_MONITOR_HAS_LIBSSH
    return std::make_shared<system_monitor::LinuxSystemMetricsProvider>(
        makeSshReader(options));
#else
    (void)options;
    throw std::runtime_error(
        "SSH target requested, but system_monitor_pwa was built without libssh");
#endif
}

[[nodiscard]] std::shared_ptr<system_monitor::SystemMetricsProvider> makeLocalProvider() {
#if defined(__APPLE__)
    return std::make_shared<system_monitor::MacOsSystemMetricsProvider>();
#elif defined(__linux__)
    auto reader = std::make_shared<system_monitor::LocalLinuxSystemReader>();
    return std::make_shared<system_monitor::LinuxSystemMetricsProvider>(reader);
#endif
}

[[nodiscard]] std::shared_ptr<system_monitor::SystemMetricsProvider> makeProvider(
    const system_monitor::RuntimeOptions& options) {
    if (options.target.type == system_monitor::MonitorTargetType::Local) {
        return makeLocalProvider();
    }
    return makeSshProvider(options.target.ssh);
}

[[nodiscard]] std::shared_ptr<system_monitor::GpioProvider> makeGpioProvider(
    const system_monitor::RuntimeOptions& options) {
    if (options.target.type == system_monitor::MonitorTargetType::Local) {
#if defined(__linux__)
        auto reader = std::make_shared<system_monitor::LocalLinuxSystemReader>();
        if (!system_monitor::hasLinuxHardwareCapability(
                *reader, system_monitor::LinuxHardwareCapability::Gpio)) {
            return nullptr;
        }
        return std::make_shared<system_monitor::GpiodGpioProvider>(reader);
#else
        return nullptr;
#endif
    }

#if SYSTEM_MONITOR_HAS_LIBSSH
    // Keep GPIO on its own SSH session. /api/system and /api/gpio can be
    // handled concurrently, while SshClient has no thread-safe contract.
    auto reader = makeSshReader(options.target.ssh);
    if (!system_monitor::hasLinuxHardwareCapability(
            *reader, system_monitor::LinuxHardwareCapability::Gpio)) {
        return nullptr;
    }
    return std::make_shared<system_monitor::GpiodGpioProvider>(reader);
#else
    return nullptr;
#endif
}

[[nodiscard]] std::shared_ptr<system_monitor::I2cProvider> makeI2cProvider(
    const system_monitor::RuntimeOptions& options) {
    if (options.target.type == system_monitor::MonitorTargetType::Local) {
#if defined(__linux__)
        auto reader = std::make_shared<system_monitor::LocalLinuxSystemReader>();
        if (!system_monitor::hasLinuxHardwareCapability(
                *reader, system_monitor::LinuxHardwareCapability::I2c)) {
            return nullptr;
        }
        return std::make_shared<system_monitor::I2cToolsProvider>(
            reader, options.i2cScanBuses);
#else
        return nullptr;
#endif
    }

#if SYSTEM_MONITOR_HAS_LIBSSH
    // Keep I2C on a dedicated SSH session because its cached inventory can
    // refresh concurrently with /api/system and /api/gpio.
    auto reader = makeSshReader(options.target.ssh);
    if (!system_monitor::hasLinuxHardwareCapability(
            *reader, system_monitor::LinuxHardwareCapability::I2c)) {
        return nullptr;
    }
    return std::make_shared<system_monitor::I2cToolsProvider>(
        reader, options.i2cScanBuses);
#else
    return nullptr;
#endif
}

[[nodiscard]] std::shared_ptr<system_monitor::SpiProvider> makeSpiProvider(
    const system_monitor::RuntimeOptions& options) {
    if (options.target.type == system_monitor::MonitorTargetType::Local) {
#if defined(__linux__)
        auto reader = std::make_shared<system_monitor::LocalLinuxSystemReader>();
        if (!system_monitor::hasLinuxHardwareCapability(
                *reader, system_monitor::LinuxHardwareCapability::Spi)) {
            return nullptr;
        }
        return std::make_shared<system_monitor::SpidevSpiProvider>(reader);
#else
        return nullptr;
#endif
    }

#if SYSTEM_MONITOR_HAS_LIBSSH
    auto reader = makeSshReader(options.target.ssh);
    if (!system_monitor::hasLinuxHardwareCapability(
            *reader, system_monitor::LinuxHardwareCapability::Spi)) {
        return nullptr;
    }
    return std::make_shared<system_monitor::SpidevSpiProvider>(reader);
#else
    return nullptr;
#endif
}

[[nodiscard]] std::shared_ptr<system_monitor::UartProvider> makeUartProvider(
    const system_monitor::RuntimeOptions& options) {
    if (options.target.type == system_monitor::MonitorTargetType::Local) {
#if defined(__linux__)
        auto reader = std::make_shared<system_monitor::LocalLinuxSystemReader>();
        if (!system_monitor::hasLinuxHardwareCapability(
                *reader, system_monitor::LinuxHardwareCapability::Uart)) {
            return nullptr;
        }
        return std::make_shared<system_monitor::LinuxUartProvider>(reader);
#else
        return nullptr;
#endif
    }

#if SYSTEM_MONITOR_HAS_LIBSSH
    // UART inventory uses its own SSH session so it can refresh independently.
    auto reader = makeSshReader(options.target.ssh);
    if (!system_monitor::hasLinuxHardwareCapability(
            *reader, system_monitor::LinuxHardwareCapability::Uart)) {
        return nullptr;
    }
    return std::make_shared<system_monitor::LinuxUartProvider>(reader);
#else
    return nullptr;
#endif
}

} // namespace

int main(int argc, const char* argv[]) {
    const auto options = system_monitor::parseRuntimeOptions(argc, argv);
    if (options.showHelp) {
        std::cout << system_monitor::runtimeOptionsHelp(argc > 0 ? argv[0] : "system_monitor_pwa");
        return 0;
    }

    drogular::App app;

    app.expressionFunction(
        "t",
        system_monitor::systemMonitorTranslationExpressionFunction());

    app.services().addFactory<drogular::TranslationProvider>(
        drogular::ServiceLifetime::Singleton,
        [] {
            return std::make_shared<system_monitor::SystemMonitorTranslations>();
        });

    app.templateRoot("examples/system_monitor_pwa/templates")
       .templateCache(false)
       .staticFiles(
           "/assets",
           "examples/system_monitor_pwa/public")
       .staticFiles(
           "/assets/icons",
           "examples/system_monitor_pwa/public/icons")
       .staticFileCacheProfile(drogular::StaticFileCacheProfile::Development)
       .serviceWorker("examples/system_monitor_pwa/public/service-worker.js")
       .offlinePage<system_monitor::OfflinePage>()
       .profile(drogular::ApplicationProfile::Development);

    auto provider = makeProvider(options);
    auto gpioProvider = makeGpioProvider(options);
    auto i2cProvider = makeI2cProvider(options);
    auto spiProvider = makeSpiProvider(options);
    auto uartProvider = makeUartProvider(options);

    app.services().registerService<system_monitor::SystemMetricsProvider>(provider);
    app.services().add<system_monitor::SystemMonitor>(provider);
    app.services().add<system_monitor::ProcessService>(provider);
    if (gpioProvider) {
        app.services().registerService<system_monitor::GpioProvider>(gpioProvider);
    }
    app.services().add<system_monitor::GpioService>(gpioProvider);
    if (i2cProvider) {
        app.services().registerService<system_monitor::I2cProvider>(i2cProvider);
    }
    app.services().add<system_monitor::I2cService>(i2cProvider);
    if (spiProvider) {
        app.services().registerService<system_monitor::SpiProvider>(spiProvider);
    }
    app.services().add<system_monitor::SpiService>(spiProvider);
    if (uartProvider) {
        app.services().registerService<system_monitor::UartProvider>(uartProvider);
    }
    app.services().add<system_monitor::UartService>(uartProvider);

    app.page<system_monitor::DashboardPage>("/");
    app.page<system_monitor::BoardPage>("/hardware");
    app.action<system_monitor::LanguageAction>("/language");
    app.get<system_monitor::SystemStatusAction>("/api/system");
    app.get<system_monitor::ProcessStatusAction>("/api/processes");
    app.get<system_monitor::GpioStatusAction>("/api/gpio");
    app.get<system_monitor::I2cStatusAction>("/api/i2c");
    app.get<system_monitor::SpiStatusAction>("/api/spi");
    app.get<system_monitor::UartStatusAction>("/api/uart");
    app.run(8080);
}