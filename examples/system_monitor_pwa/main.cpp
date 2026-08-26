#include "actions/system_status_action.hpp"
#include "pages/dashboard_page.hpp"
#if defined(__APPLE__)
#include "platform/macos_system_metrics_provider.hpp"
#elif defined(__linux__)
#include "platform/linux_system_metrics_provider.hpp"
#include "platform/local_linux_system_reader.hpp"
#else
#error "system_monitor_pwa currently supports macOS and Linux"
#endif
#include "services/system_monitor.hpp"
#include "system/system_metrics_provider.hpp"

#include <drogular/app.hpp>
#include <drogular/static_file_cache_profile.hpp>

#include <memory>

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
    auto reader = std::make_shared<system_monitor::LocalLinuxSystemReader>();
    auto provider = std::make_shared<system_monitor::LinuxSystemMetricsProvider>(reader);
#endif
    app.services().registerService<system_monitor::SystemMetricsProvider>(provider);
    app.services().add<system_monitor::SystemMonitor>(provider);

    app.page<system_monitor::DashboardPage>("/");
    app.get<system_monitor::SystemStatusAction>("/api/system");
    app.run(8080);
}