#include "actions/system_status_action.hpp"
#include "pages/dashboard_page.hpp"
#if defined(__APPLE__)
#include "platform/macos_system_metrics_provider.hpp"
#elif defined(__linux__)
#include "platform/linux_system_metrics_provider.hpp"
#include "platform/local_linux_system_reader.hpp"
#if SYSTEM_MONITOR_HAS_LIBSSH
#include "ssh/libssh_client.hpp"
#include "ssh/ssh_system_reader.hpp"
#endif
#else
#error "system_monitor_pwa currently supports macOS and Linux"
#endif
#include "services/system_monitor.hpp"
#include "system/system_metrics_provider.hpp"

#include <drogular/app.hpp>
#include <drogular/static_file_cache_profile.hpp>

#include <charconv>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

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

#if defined(__linux__)
[[nodiscard]] std::shared_ptr<system_monitor::SystemMetricsProvider> makeLinuxProvider() {
    const char* target = environmentValue("SYSTEM_MONITOR_TARGET");
    if (target == nullptr || std::string_view(target) == "local") {
        auto reader = std::make_shared<system_monitor::LocalLinuxSystemReader>();
        return std::make_shared<system_monitor::LinuxSystemMetricsProvider>(reader);
    }

    if (std::string_view(target) != "ssh") {
        throw std::invalid_argument("SYSTEM_MONITOR_TARGET must be 'local' or 'ssh'");
    }

#if SYSTEM_MONITOR_HAS_LIBSSH
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

    auto client = std::make_shared<system_monitor::LibsshClient>();
    client->connect(options);
    auto reader = std::make_shared<system_monitor::SshSystemReader>(client);
    return std::make_shared<system_monitor::LinuxSystemMetricsProvider>(reader);
#else
    throw std::runtime_error(
        "SYSTEM_MONITOR_TARGET=ssh requested, but system_monitor_pwa was built without libssh");
#endif
}
#endif

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

#if defined(__APPLE__)
    auto provider = std::make_shared<system_monitor::MacOsSystemMetricsProvider>();
#elif defined(__linux__)
    auto provider = makeLinuxProvider();
#endif
    app.services().registerService<system_monitor::SystemMetricsProvider>(provider);
    app.services().add<system_monitor::SystemMonitor>(provider);

    app.page<system_monitor::DashboardPage>("/");
    app.get<system_monitor::SystemStatusAction>("/api/system");
    app.run(8080);
}