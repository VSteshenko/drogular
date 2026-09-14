#include "localization/system_monitor_translations.hpp"

#include <drogular/interactions_resources.hpp>

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
    const auto systemFragment = readFile(root / "fragments/system.html");
    const auto offline = readFile(root / "offline.html");

    EXPECT_NE(layout.find("{{ locale }}"), std::string::npos);
    EXPECT_NE(layout.find("action=\"/language\""), std::string::npos);
    EXPECT_EQ(layout.find("class=\"site-nav\""), std::string::npos);
    EXPECT_NE(layout.find("language-button is-active"), std::string::npos);
    EXPECT_EQ(layout.find("system-monitor-i18n"), std::string::npos);
    EXPECT_NE(dashboard.find("dg-get=\"/fragments/system\""), std::string::npos);
    EXPECT_NE(systemFragment.find("t(\"dashboard.monitoring_target\")"), std::string::npos);
    EXPECT_NE(systemFragment.find("data-dg-connection-label"), std::string::npos);
    EXPECT_NE(board.find("data-board-live-region"), std::string::npos);
    EXPECT_NE(board.find("dg-get=\"/fragments/hardware\""), std::string::npos);
    EXPECT_NE(offline.find("t(\"offline.title\")"), std::string::npos);
}

TEST(SystemMonitorLocalizationTests, ClientScriptsUseServerRenderedTranslations) {
    const auto root = std::filesystem::path(DROGULAR_SOURCE_DIR) /
                      "examples/system_monitor_pwa/public";
    const auto interactions = drogular::interactions_resources::script();
    const auto worker = readFile(root / "service-worker.js");

    EXPECT_NE(interactions.find("dataset.dgConnectionLabel"), std::string::npos);
    EXPECT_NE(interactions.find("dgLabelReconnecting"), std::string::npos);
    EXPECT_NE(interactions.find("dg-pause-on-failure"), std::string::npos);
    EXPECT_NE(interactions.find("window.clearInterval(interval)"), std::string::npos);
    EXPECT_NE(interactions.find("pollGroupElements(element).forEach(pauseElement)"), std::string::npos);
    EXPECT_NE(interactions.find("members.forEach((member) => refresh(member))"), std::string::npos);
    EXPECT_NE(interactions.find("document.querySelectorAll('[dg-resume]')"), std::string::npos);
    EXPECT_NE(interactions.find("new CustomEvent('dg:resume')"), std::string::npos);
    EXPECT_NE(worker.find("drogular-system-monitor-v22"), std::string::npos);
    EXPECT_EQ(worker.find("/assets/board.js"), std::string::npos);
    EXPECT_NE(worker.find("fetch(OFFLINE_PAGE)"), std::string::npos);
}