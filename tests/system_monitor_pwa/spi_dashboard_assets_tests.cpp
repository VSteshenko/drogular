#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

#ifndef DROGULAR_SOURCE_DIR
#error DROGULAR_SOURCE_DIR missing
#endif

static std::string read(const char* path) {
    std::ifstream in(std::string(DROGULAR_SOURCE_DIR) + path);
    std::ostringstream out;
    out << in.rdbuf();

    return out.str();
}

TEST(SpiDashboardAssetsTests, UsesServerFragment) {
    auto html = read("/examples/system_monitor_pwa/templates/dashboard.html");
    auto fragment = read("/examples/system_monitor_pwa/templates/fragments/spi.html");

    EXPECT_NE(html.find("dg-get=\"/fragments/spi\""), std::string::npos);
    EXPECT_NE(html.find("dg-trigger=\"load, every 30s\""), std::string::npos);
    EXPECT_NE(fragment.find("spi-gpio-pin"), std::string::npos);
    EXPECT_NE(fragment.find("spi-device-list"), std::string::npos);
}