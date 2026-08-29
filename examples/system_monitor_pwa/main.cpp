#include "actions/gpio_status_action.hpp"
#include "actions/i2c_status_action.hpp"
#include "actions/system_status_action.hpp"
#include "gpio/gpiod_gpio_provider.hpp"
#include "i2c/i2c_tools_provider.hpp"
#include "pages/dashboard_page.hpp"
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
#include "services/system_monitor.hpp"
#include "system/monitor_target.hpp"
#include "system/system_metrics_provider.hpp"

#include <drogular/app.hpp>
#include <drogular/static_file_cache_profile.hpp>

#include <charconv>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

[[nodiscard]] const char* environmentValue(const char* name) noexcept {
    const char* value = std::getenv(name);
    return value != nullptr && *value != '\0' ? value : nullptr;
}

[[nodiscard]] std::uint16_t sshPortFromEnvironment() {
    const char* value = environmentValue("SYSTEM_MONITOR_SSH_PORT");
    if (value == nullptr) {
        return 22;
    }

    unsigned int port = 0;
    const std::string_view text(value);
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), port);
    if (error != std::errc{} || end != text.data() + text.size() || port == 0 || port > 65535) {
        throw std::invalid_argument("SYSTEM_MONITOR_SSH_PORT must be between 1 and 65535");
    }

    return static_cast<std::uint16_t>(port);
}


[[nodiscard]] std::vector<std::uint32_t> i2cScanBusesFromEnvironment() {
    const char* value = environmentValue("SYSTEM_MONITOR_I2C_SCAN_BUSES");
    if (value == nullptr) {
        return {};
    }

    std::vector<std::uint32_t> buses;
    std::string_view remaining(value);
    while (!remaining.empty()) {
        const auto comma = remaining.find(',');
        auto token = remaining.substr(0, comma);
        const auto first = token.find_first_not_of(" \t");
        const auto last = token.find_last_not_of(" \t");
        if (first == std::string_view::npos) {
            throw std::invalid_argument(
                "SYSTEM_MONITOR_I2C_SCAN_BUSES contains an empty bus number");
        }
        token = token.substr(first, last - first + 1);

        std::uint32_t bus = 0;
        const auto [end, error] = std::from_chars(
            token.data(), token.data() + token.size(), bus);
        if (error != std::errc{} || end != token.data() + token.size()) {
            throw std::invalid_argument(
                "SYSTEM_MONITOR_I2C_SCAN_BUSES must be a comma-separated list of bus numbers");
        }
        buses.push_back(bus);

        if (comma == std::string_view::npos) {
            break;
        }
        remaining.remove_prefix(comma + 1);
    }

    return buses;
}

[[nodiscard]] system_monitor::SshTargetOptions sshOptionsFromEnvironment() {
    system_monitor::SshTargetOptions options;

    const char* host = environmentValue("SYSTEM_MONITOR_SSH_HOST");
    const char* user = environmentValue("SYSTEM_MONITOR_SSH_USER");
    const char* identityFile = environmentValue("SYSTEM_MONITOR_SSH_IDENTITY_FILE");
    const char* knownHostsFile = environmentValue("SYSTEM_MONITOR_SSH_KNOWN_HOSTS_FILE");
    if (host == nullptr || user == nullptr || identityFile == nullptr || knownHostsFile == nullptr) {
        throw std::invalid_argument(
            "SSH target requires SYSTEM_MONITOR_SSH_HOST, SYSTEM_MONITOR_SSH_USER, "
            "SYSTEM_MONITOR_SSH_IDENTITY_FILE and SYSTEM_MONITOR_SSH_KNOWN_HOSTS_FILE");
    }

    options.host = host;
    options.port = sshPortFromEnvironment();
    options.user = user;
    options.identityFile = identityFile;
    options.knownHostsFile = knownHostsFile;
    return options;
}

#if SYSTEM_MONITOR_HAS_LIBSSH
[[nodiscard]] std::shared_ptr<system_monitor::SshSystemReader> makeSshReader() {
    auto client = std::make_shared<system_monitor::LibsshClient>();
    client->connect(sshOptionsFromEnvironment());
    return std::make_shared<system_monitor::SshSystemReader>(client);
}
#endif

[[nodiscard]] std::shared_ptr<system_monitor::SystemMetricsProvider> makeSshProvider() {
#if SYSTEM_MONITOR_HAS_LIBSSH
    return std::make_shared<system_monitor::LinuxSystemMetricsProvider>(
        makeSshReader());
#else
    throw std::runtime_error(
        "SYSTEM_MONITOR_TARGET=ssh requested, but system_monitor_pwa was built without libssh");
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

[[nodiscard]] std::shared_ptr<system_monitor::SystemMetricsProvider> makeProvider() {
    const char* target = environmentValue("SYSTEM_MONITOR_TARGET");
    if (target == nullptr || std::string_view(target) == "local") {
        return makeLocalProvider();
    }

    if (std::string_view(target) == "ssh") {
        return makeSshProvider();
    }

    throw std::invalid_argument("SYSTEM_MONITOR_TARGET must be 'local' or 'ssh'");
}

[[nodiscard]] std::shared_ptr<system_monitor::GpioProvider> makeGpioProvider() {
    const char* target = environmentValue("SYSTEM_MONITOR_TARGET");
    if (target == nullptr || std::string_view(target) == "local") {
#if defined(__linux__)
        auto reader = std::make_shared<system_monitor::LocalLinuxSystemReader>();
        return std::make_shared<system_monitor::GpiodGpioProvider>(reader);
#else
        return nullptr;
#endif
    }

    if (std::string_view(target) == "ssh") {
#if SYSTEM_MONITOR_HAS_LIBSSH
        // Keep GPIO on its own SSH session. /api/system and /api/gpio can be
        // handled concurrently, while SshClient has no thread-safe contract.
        return std::make_shared<system_monitor::GpiodGpioProvider>(
            makeSshReader());
#else
        return nullptr;
#endif
    }

    throw std::invalid_argument("SYSTEM_MONITOR_TARGET must be 'local' or 'ssh'");
}


[[nodiscard]] std::shared_ptr<system_monitor::I2cProvider> makeI2cProvider() {
    const char* target = environmentValue("SYSTEM_MONITOR_TARGET");
    if (target == nullptr || std::string_view(target) == "local") {
#if defined(__linux__)
        auto reader = std::make_shared<system_monitor::LocalLinuxSystemReader>();
        return std::make_shared<system_monitor::I2cToolsProvider>(reader, i2cScanBusesFromEnvironment());
#else
        return nullptr;
#endif
    }

    if (std::string_view(target) == "ssh") {
#if SYSTEM_MONITOR_HAS_LIBSSH
        // Keep I2C on a dedicated SSH session because its cached inventory can
        // refresh concurrently with /api/system and /api/gpio.
        return std::make_shared<system_monitor::I2cToolsProvider>(
            makeSshReader(),
            i2cScanBusesFromEnvironment());
#else
        return nullptr;
#endif
    }

    throw std::invalid_argument("SYSTEM_MONITOR_TARGET must be 'local' or 'ssh'");
}

} // namespace

int main() {
    drogular::App app;

    app.templateRoot("examples/system_monitor_pwa/templates")
       .templateCache(false)
       .staticFiles(
           "/assets",
           "examples/system_monitor_pwa/public")
       .staticFileCacheProfile(drogular::StaticFileCacheProfile::Development)
       .profile(drogular::ApplicationProfile::Development);

    auto provider = makeProvider();
    auto gpioProvider = makeGpioProvider();
    auto i2cProvider = makeI2cProvider();

    app.services().registerService<system_monitor::SystemMetricsProvider>(provider);
    app.services().add<system_monitor::SystemMonitor>(provider);
    if (gpioProvider) {
        app.services().registerService<system_monitor::GpioProvider>(gpioProvider);
    }
    app.services().add<system_monitor::GpioService>(gpioProvider);
    if (i2cProvider) {
        app.services().registerService<system_monitor::I2cProvider>(i2cProvider);
    }
    app.services().add<system_monitor::I2cService>(i2cProvider);

    app.page<system_monitor::DashboardPage>("/");
    app.get<system_monitor::SystemStatusAction>("/api/system");
    app.get<system_monitor::GpioStatusAction>("/api/gpio");
    app.get<system_monitor::I2cStatusAction>("/api/i2c");
    app.run(8080);
}
