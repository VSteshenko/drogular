#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

namespace {

std::string readFile(const std::string& path) {
    std::ifstream stream(path); std::ostringstream out; out << stream.rdbuf(); return out.str();
}

}

TEST(ProcessDashboardAssetsTests, ContainsReadOnlyProcessInventoryWithFilteringAndSorting) {
    const auto html = readFile(std::string(DROGULAR_SOURCE_DIR) +
        "/examples/system_monitor_pwa/templates/dashboard.html");
    const auto js = readFile(std::string(DROGULAR_SOURCE_DIR) +
        "/examples/system_monitor_pwa/public/app.js");

    EXPECT_NE(html.find("data-process-panel"), std::string::npos);
    EXPECT_NE(html.find("data-process-search"), std::string::npos);
    EXPECT_NE(html.find("dg-get=\"/fragments/processes\""), std::string::npos);
    EXPECT_NE(html.find("dg-trigger=\"load, every 3s, input delay:250ms, change\""), std::string::npos);
    EXPECT_NE(html.find("data-process-sort"), std::string::npos);
    EXPECT_NE(html.find("t(\"dashboard.process_note\")"), std::string::npos);
    EXPECT_EQ(js.find("fetch('/api/processes'"), std::string::npos);
    EXPECT_EQ(js.find("renderProcesses"), std::string::npos);

    const auto interactions = readFile(std::string(DROGULAR_SOURCE_DIR) +
        "/examples/system_monitor_pwa/public/interactions.js");
    EXPECT_NE(interactions.find("document.querySelectorAll('[dg-get]')"), std::string::npos);
    EXPECT_NE(interactions.find("Accept': 'text/html"), std::string::npos);
}