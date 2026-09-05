#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

namespace {

std::string readSource(const std::string& relativePath) {
    std::ifstream input(std::string(DROGULAR_SOURCE_DIR) + "/" + relativePath);
    std::ostringstream output;
    output << input.rdbuf();

    return output.str();
}

}

TEST(I2cDashboardAssetsTests, UsesServerFragment) {
    const auto html = readSource(
        "examples/system_monitor_pwa/templates/dashboard.html");
    const auto fragment = readSource(
        "examples/system_monitor_pwa/templates/fragments/i2c.html");

    EXPECT_NE(html.find("data-i2c-panel"), std::string::npos);
    EXPECT_NE(html.find("dg-get=\"/fragments/i2c\""), std::string::npos);
    EXPECT_NE(html.find("dg-trigger=\"load, every 60s\""), std::string::npos);
    EXPECT_NE(fragment.find("i2c-gpio-pin"), std::string::npos);
    EXPECT_NE(fragment.find("i2c-address-list"), std::string::npos);
}