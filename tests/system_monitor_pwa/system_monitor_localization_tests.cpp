#include "localization/system_monitor_translations.hpp"

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

TEST(SystemMonitorLocalizationTests, ProvidesEnglishAndGermanTranslations) {
    system_monitor::SystemMonitorTranslations translations;

    EXPECT_EQ(
        translations.translate("en", "dashboard.monitoring_target"),
        "Monitoring target");
    EXPECT_EQ(
        translations.translate("de", "dashboard.monitoring_target"),
        "Überwachungsziel");
    EXPECT_EQ(
        translations.translate("de", "offline.title"),
        "Sie sind offline");
    EXPECT_EQ(
        translations.translate("xx", "status.retry"),
        "Retry");
}

TEST(SystemMonitorLocalizationTests, TemplatesUseTranslationExpression) {
    const auto root = std::filesystem::path(DROGULAR_SOURCE_DIR) /
                      "examples/system_monitor_pwa/templates";
    const auto layout = readFile(root / "layouts/main.html");
    const auto dashboard = readFile(root / "dashboard.html");
    const auto board = readFile(root / "board.html");
    const auto offline = readFile(root / "offline.html");

    EXPECT_NE(layout.find("{{ locale }}"), std::string::npos);
    EXPECT_NE(layout.find("action=\"/language\""), std::string::npos);
    EXPECT_NE(layout.find("system-monitor-i18n"), std::string::npos);
    EXPECT_NE(dashboard.find("t(\"dashboard.monitoring_target\")"), std::string::npos);
    EXPECT_NE(board.find("t(\"board.overview\")"), std::string::npos);
    EXPECT_NE(offline.find("t(\"offline.title\")"), std::string::npos);
}

TEST(SystemMonitorLocalizationTests, ClientScriptsUseServerRenderedTranslations) {
    const auto root = std::filesystem::path(DROGULAR_SOURCE_DIR) /
                      "examples/system_monitor_pwa/public";
    const auto app = readFile(root / "app.js");
    const auto board = readFile(root / "board.js");
    const auto worker = readFile(root / "service-worker.js");

    EXPECT_NE(app.find("system-monitor-i18n"), std::string::npos);
    EXPECT_NE(app.find("tr('status.live'"), std::string::npos);
    EXPECT_NE(board.find("system-monitor-i18n"), std::string::npos);
    EXPECT_NE(board.find("tr('status.offline'"), std::string::npos);
    EXPECT_NE(worker.find("drogular-system-monitor-v4"), std::string::npos);
    EXPECT_NE(worker.find("fetch(OFFLINE_PAGE)"), std::string::npos);
}