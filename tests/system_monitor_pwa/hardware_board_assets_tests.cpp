#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

std::string readAsset(const std::filesystem::path& path) {
    std::ifstream stream(path);
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

} // namespace

TEST(HardwareBoardAssetsTests, ComposesExistingHardwareApisIntoPhysicalHeaderMap) {
    const auto root = std::filesystem::path(DROGULAR_SOURCE_DIR) /
                      "examples/system_monitor_pwa";
    const auto html = readAsset(root / "templates/board.html");
    const auto js = readAsset(root / "public/board.js");
    const auto css = readAsset(root / "public/app.css");

    EXPECT_NE(html.find("data-board-header-map"), std::string::npos);
    EXPECT_NE(html.find("data-board-interface-list"), std::string::npos);
    EXPECT_NE(html.find("40-pin header"), std::string::npos);

    EXPECT_NE(js.find("fetchJson('/api/system')"), std::string::npos);
    EXPECT_NE(js.find("fetchJson('/api/gpio')"), std::string::npos);
    EXPECT_NE(js.find("fetchJson('/api/i2c')"), std::string::npos);
    EXPECT_NE(js.find("fetchJson('/api/spi')"), std::string::npos);
    EXPECT_NE(js.find("fetchJson('/api/uart')"), std::string::npos);
    EXPECT_NE(js.find("physicalHeaderPin"), std::string::npos);
    EXPECT_NE(js.find("hasPhysicalHeader"), std::string::npos);
    EXPECT_NE(js.find("`${gpioLines} GPIO lines`"), std::string::npos);
    EXPECT_NE(js.find("pinBadge(pin, pin.role)"), std::string::npos);
    EXPECT_NE(js.find("I²C${bus.number}"), std::string::npos);
    EXPECT_NE(js.find("SPI${bus.number}"), std::string::npos);
    EXPECT_NE(js.find("UART${group.controller}"), std::string::npos);

    EXPECT_NE(css.find(".board-header-map"), std::string::npos);
    EXPECT_NE(css.find(".hardware-summary-grid"), std::string::npos);
    EXPECT_NE(css.find(".board-interface-card"), std::string::npos);
    EXPECT_NE(css.find(".board-overview-grid-generic"), std::string::npos);
}