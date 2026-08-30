#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

#ifndef DROGULAR_SOURCE_DIR
#error DROGULAR_SOURCE_DIR missing
#endif

static std::string read(const char* path) {
    std::ifstream file(std::string(DROGULAR_SOURCE_DIR) + path);
    std::ostringstream stream;
    stream << file.rdbuf();

    return stream.str();
}

TEST(SpiDashboardAssetsTests, ContainsSpiPanelAndPolling) {
    auto html = read("/examples/system_monitor_pwa/templates/dashboard.html");
    auto js = read("/examples/system_monitor_pwa/public/app.js");

    EXPECT_NE(html.find("data-spi-panel"), std::string::npos);
    EXPECT_NE(js.find("fetch('/api/spi'"), std::string::npos);
    EXPECT_NE(js.find("pollSpi();"),std::string::npos);
    EXPECT_NE(js.find("pin.physicalHeaderPin"), std::string::npos);
    EXPECT_NE(js.find("pin.exposure"), std::string::npos);
}