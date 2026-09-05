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

    EXPECT_NE(html.find("data-process-panel"), std::string::npos);
    EXPECT_NE(html.find("data-process-search"), std::string::npos);
    EXPECT_NE(html.find("dg-get=\"/fragments/processes\""), std::string::npos);
    EXPECT_NE(html.find("dg-trigger=\"load, every 3s, input delay:250ms, change\""), std::string::npos);
    EXPECT_NE(html.find("dg-poll-group=\"monitor\""), std::string::npos);
    EXPECT_NE(html.find("data-process-sort"), std::string::npos);
    EXPECT_NE(html.find("data-dg-state=\"loading\""), std::string::npos);
    EXPECT_NE(html.find("data-dg-state-view=\"loading\""), std::string::npos);
    EXPECT_NE(html.find("data-dg-state-view=\"error\""), std::string::npos);
    EXPECT_NE(html.find("t(\"dashboard.process_note\")"), std::string::npos);

    const auto interactions = readFile(std::string(DROGULAR_SOURCE_DIR) +
        "/examples/system_monitor_pwa/public/interactions.js");
    EXPECT_NE(interactions.find("document.querySelectorAll('[dg-get]')"), std::string::npos);
    EXPECT_NE(interactions.find("Accept': 'text/html"), std::string::npos);
    EXPECT_NE(interactions.find("setState(element, 'loading')"), std::string::npos);
    EXPECT_NE(interactions.find("setState(element, 'error')"), std::string::npos);
    EXPECT_NE(interactions.find("querySelector('[data-dg-empty]')"), std::string::npos);
    EXPECT_NE(interactions.find("aria-busy"), std::string::npos);

    const auto css = readFile(std::string(DROGULAR_SOURCE_DIR) +
        "/examples/system_monitor_pwa/public/app.css");
    EXPECT_NE(css.find(".dg-empty-state"), std::string::npos);
    EXPECT_NE(css.find("[data-dg-state-view=\"loading\"]"), std::string::npos);
    EXPECT_NE(css.find("[data-dg-state-view=\"error\"]"), std::string::npos);
}