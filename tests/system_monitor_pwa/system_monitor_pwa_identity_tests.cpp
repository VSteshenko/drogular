#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

std::string readAsset(const std::filesystem::path& path) {
    std::ifstream stream(path);
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

}

TEST(SystemMonitorPwaIdentityTests, ManifestDefinesInstallableApplicationIcons) {
    const auto root = std::filesystem::path(DROGULAR_SOURCE_DIR) /
                      "examples/system_monitor_pwa";
    const auto manifest = readAsset(root / "public/manifest.webmanifest");

    EXPECT_NE(manifest.find("\"name\": \"Drogular System Monitor\""), std::string::npos);
    EXPECT_NE(manifest.find("\"short_name\": \"System Monitor\""), std::string::npos);
    EXPECT_NE(manifest.find("\"id\": \"/\""), std::string::npos);
    EXPECT_NE(manifest.find("icon-192.png"), std::string::npos);
    EXPECT_NE(manifest.find("icon-512.png"), std::string::npos);
    EXPECT_NE(manifest.find("icon-512-maskable.png"), std::string::npos);
    EXPECT_NE(manifest.find("\"purpose\": \"maskable\""), std::string::npos);
    EXPECT_NE(manifest.find("\"form_factor\": \"wide\""), std::string::npos);
    EXPECT_NE(manifest.find("screenshot-wide.png"), std::string::npos);
    EXPECT_NE(manifest.find("screenshot-mobile.png"), std::string::npos);

    EXPECT_TRUE(std::filesystem::exists(root / "public/icons/icon-192.png"));
    EXPECT_TRUE(std::filesystem::exists(root / "public/icons/icon-512.png"));
    EXPECT_TRUE(std::filesystem::exists(root / "public/icons/icon-512-maskable.png"));
    EXPECT_TRUE(std::filesystem::exists(root / "public/icons/apple-touch-icon.png"));
    EXPECT_TRUE(std::filesystem::exists(root / "public/icons/favicon.svg"));
    EXPECT_TRUE(std::filesystem::exists(root / "public/screenshot-wide.png"));
    EXPECT_TRUE(std::filesystem::exists(root / "public/screenshot-mobile.png"));
}

TEST(SystemMonitorPwaIdentityTests, LayoutAndServiceWorkerExposeIdentityAssets) {
    const auto root = std::filesystem::path(DROGULAR_SOURCE_DIR) /
                      "examples/system_monitor_pwa";
    const auto layout = readAsset(root / "templates/layouts/main.html");
    const auto worker = readAsset(root / "public/service-worker.js");

    EXPECT_NE(layout.find("rel=\"icon\""), std::string::npos);
    EXPECT_NE(layout.find("rel=\"apple-touch-icon\""), std::string::npos);
    EXPECT_NE(worker.find("drogular-system-monitor-v5"), std::string::npos);
    EXPECT_NE(worker.find("/assets/icons/icon-192.png"), std::string::npos);
    EXPECT_NE(worker.find("/assets/icons/icon-512-maskable.png"), std::string::npos);
}