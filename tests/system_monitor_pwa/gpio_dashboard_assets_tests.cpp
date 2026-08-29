#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

std::string readSource(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("failed to read " + path.string());
    }

    std::ostringstream output;
    output << input.rdbuf();
    return output.str();
}

} // namespace

TEST(GpioDashboardAssetsTests, PollsDedicatedGpioEndpointAtServiceCadence) {
    const auto script = readSource(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/public/app.js");

    EXPECT_NE(script.find("const GPIO_POLL_INTERVAL_MS = 30000;"),
              std::string::npos);
    EXPECT_NE(script.find("fetch('/api/gpio'"), std::string::npos);
    EXPECT_NE(script.find("window.setTimeout(pollGpio, GPIO_POLL_INTERVAL_MS)"),
              std::string::npos);
}

TEST(GpioDashboardAssetsTests, RendersNativeCollapsibleChipSections) {
    const auto script = readSource(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/public/app.js");

    EXPECT_NE(script.find("document.createElement('details')"),
              std::string::npos);
    EXPECT_NE(script.find("document.createElement('summary')"),
              std::string::npos);
    EXPECT_NE(script.find("details.open = hadRenderedChips"),
              std::string::npos);
    EXPECT_NE(script.find(": index === 0;"), std::string::npos);
}

TEST(GpioDashboardAssetsTests, KeepsSystemAndGpioPollingIndependent) {
    const auto script = readSource(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/public/app.js");

    EXPECT_NE(script.find("const POLL_INTERVAL_MS = 2000;"),
              std::string::npos);
    EXPECT_NE(script.find("const GPIO_POLL_INTERVAL_MS = 30000;"),
              std::string::npos);
    EXPECT_NE(script.find("fetch('/api/system'"), std::string::npos);
    EXPECT_NE(script.find("fetch('/api/gpio'"), std::string::npos);
}

TEST(GpioDashboardAssetsTests, HidesGpioPanelWhenPlatformIsUnavailable) {
    const auto script = readSource(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/public/app.js");

    EXPECT_NE(script.find("if (!data.available)"),
              std::string::npos);
    EXPECT_NE(script.find("gpioPanel.hidden = true;"),
              std::string::npos);
}

TEST(GpioDashboardAssetsTests, DefaultsToActiveLinesAndFiltersClientSide) {
    const auto script = readSource(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/public/app.js");

    EXPECT_NE(script.find("let gpioFilter = 'active';"),
              std::string::npos);
    EXPECT_NE(script.find("line.used"),
              std::string::npos);
    EXPECT_NE(script.find("lines.filter(gpioLineMatchesFilter)"),
              std::string::npos);
    EXPECT_NE(script.find("renderGpio(gpioData)"),
              std::string::npos);
}

TEST(GpioDashboardAssetsTests, OffersAllActiveAndFreeFilters) {
    const auto page = readSource(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/templates/dashboard.html");

    EXPECT_NE(page.find("data-gpio-filter=\"all\""),
              std::string::npos);
    EXPECT_NE(page.find("data-gpio-filter=\"active\""),
              std::string::npos);
    EXPECT_NE(page.find("data-gpio-filter=\"free\""),
              std::string::npos);
    EXPECT_NE(page.find("aria-pressed=\"true\">Active</button>"),
              std::string::npos);
}