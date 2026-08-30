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

} // namespace

TEST(I2cDashboardAssetsTests, DashboardContainsI2cPanel) {
    const auto html = readSource(
        "examples/system_monitor_pwa/templates/dashboard.html");

    EXPECT_NE(html.find("data-i2c-panel"), std::string::npos);
    EXPECT_NE(html.find("data-i2c-summary"), std::string::npos);
    EXPECT_NE(html.find("data-i2c-buses"), std::string::npos);
    EXPECT_NE(html.find(">I²C</h2>"), std::string::npos);
}

TEST(I2cDashboardAssetsTests, JavaScriptPollsIndependentI2cEndpoint) {
    const auto script = readSource(
        "examples/system_monitor_pwa/public/app.js");

    EXPECT_NE(script.find("fetch('/api/i2c'"), std::string::npos);
    EXPECT_NE(script.find("I2C_POLL_INTERVAL_MS = 60000"), std::string::npos);
    EXPECT_NE(script.find("pollI2c();"), std::string::npos);
    EXPECT_NE(script.find("claimedByKernel"), std::string::npos);
    EXPECT_NE(script.find("bus.gpioPins"), std::string::npos);
    EXPECT_NE(script.find("i2c-gpio-pin"), std::string::npos);
    EXPECT_NE(script.find("pin.physicalHeaderPin"), std::string::npos);
    EXPECT_NE(script.find("pin.exposure"), std::string::npos);
}