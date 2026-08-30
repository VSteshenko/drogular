#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

namespace {

std::string readFile(const std::string& path) {
    std::ifstream stream(path); std::ostringstream out; out << stream.rdbuf(); return out.str();
}

} // namespace

TEST(UartDashboardAssetsTests, ContainsUartPanelAndIndependentPolling) {
    const auto html = readFile(
        std::string(DROGULAR_SOURCE_DIR) + "/examples/system_monitor_pwa/templates/dashboard.html"
    );
    const auto js = readFile(
        std::string(DROGULAR_SOURCE_DIR) + "/examples/system_monitor_pwa/public/app.js"
    );
    EXPECT_NE(html.find("data-uart-panel"), std::string::npos);
    EXPECT_NE(js.find("fetch('/api/uart'"), std::string::npos);
    EXPECT_NE(js.find("UART_POLL_INTERVAL_MS = 30000"), std::string::npos);
    EXPECT_NE(js.find("gpioGroups"),std::string::npos);
    EXPECT_NE(js.find("Linux tty numbering is not assumed"), std::string::npos);
}