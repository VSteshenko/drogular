#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

static std::string readFile(const std::string& path) {
    std::ifstream in(path);
    std::ostringstream out;
    out << in.rdbuf();

    return out.str();
}

TEST(UartDashboardAssetsTests, UsesServerFragment) {
    const auto base = std::string(DROGULAR_SOURCE_DIR);
    auto html = readFile(
        base+"/examples/system_monitor_pwa/templates/dashboard.html");
    auto js = readFile(
        base+"/examples/system_monitor_pwa/public/app.js");
    auto fragment = readFile(
        base+"/examples/system_monitor_pwa/templates/fragments/uart.html");

    EXPECT_NE(html.find("dg-get=\"/fragments/uart\""), std::string::npos);
    EXPECT_NE(html.find("dg-trigger=\"load, every 30s\""), std::string::npos);
    EXPECT_EQ(js.find("fetch('/api/uart'"), std::string::npos);
    EXPECT_NE(fragment.find("uart-exposure"), std::string::npos);
    EXPECT_NE(fragment.find("uart-gpio-pin"), std::string::npos);
}