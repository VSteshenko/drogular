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
    EXPECT_NE(html.find("data-process-sort"), std::string::npos);
    EXPECT_NE(html.find("Inventory is read-only"), std::string::npos);
    EXPECT_NE(js.find("fetch('/api/processes'"), std::string::npos);
    EXPECT_NE(js.find("PROCESS_POLL_INTERVAL_MS = 3000"), std::string::npos);
    EXPECT_NE(js.find("PROCESS_LIMIT = 50"), std::string::npos);
}