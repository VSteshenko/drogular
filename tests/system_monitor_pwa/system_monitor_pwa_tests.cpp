#include "pages/offline_page.hpp"

#include <drogular/services.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

std::string readFile(const std::filesystem::path& path) {
    std::ifstream stream(path);
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

} // namespace

TEST(SystemMonitorPwaTests, ServiceWorkerNeverCachesMonitoringApisOrRenderedSnapshots) {
    const auto root = std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa";
    const auto worker = readFile(root / "public/service-worker.js");

    EXPECT_NE(worker.find("url.pathname.startsWith(\"/api/\")"), std::string::npos);
    EXPECT_NE(worker.find("fetch(event.request)"), std::string::npos);
    EXPECT_NE(worker.find("caches.match(OFFLINE_PAGE)"), std::string::npos);
    EXPECT_EQ(worker.find("\"/\",\n"), std::string::npos);
    EXPECT_EQ(worker.find("\"/hardware\""), std::string::npos);
}

TEST(SystemMonitorPwaTests, ManifestDefinesStandaloneSystemMonitorShell) {
    const auto manifest = readFile(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/public/manifest.webmanifest");

    EXPECT_NE(manifest.find("\"name\": \"Drogular System Monitor\""), std::string::npos);
    EXPECT_NE(manifest.find("\"display\": \"standalone\""), std::string::npos);
    EXPECT_NE(manifest.find("\"start_url\": \"/\""), std::string::npos);
    EXPECT_NE(manifest.find("\"theme_color\": \"#10172a\""), std::string::npos);
}

TEST(SystemMonitorPwaTests, OfflinePageExplainsThatMetricsAreNotCached) {
    drogular::ApplicationServices services;
    drogular::ApplicationOptions options;
    options.setTemplateRoot(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/templates");
    services.setOptions(&options);

    const auto result = drogular::test::renderPage<system_monitor::OfflinePage>(&services);

    EXPECT_TRUE(drogular::test::contains(result.html, "You are offline"));
    EXPECT_TRUE(drogular::test::contains(result.html, "No cached metrics are shown"));
    EXPECT_TRUE(drogular::test::contains(result.html, "rel=\"manifest\""));
    EXPECT_TRUE(drogular::test::contains(result.html, "navigator.serviceWorker.register"));
    EXPECT_TRUE(drogular::test::contains(result.html, "status status-offline"));
    EXPECT_FALSE(drogular::test::contains(result.html, "src=\"/assets/app.js\""));
}