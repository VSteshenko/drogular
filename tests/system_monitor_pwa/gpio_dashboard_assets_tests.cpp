#include <drogular/interactions_resources.hpp>

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

TEST(GpioDashboardAssetsTests, UsesDeclarativeFragmentAtServiceCadence) {
    const auto page = readSource(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/templates/dashboard.html");

    EXPECT_NE(page.find("dg-get=\"/fragments/gpio\""), std::string::npos);
    EXPECT_NE(page.find("dg-trigger=\"load, every 30s, change\""), std::string::npos);
    EXPECT_NE(page.find("dg-target=\"[data-dg-fragment]\""), std::string::npos);
}

TEST(GpioDashboardAssetsTests, OffersServerDrivenAllActiveAndFreeFilters) {
    const auto fragment = readSource(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/templates/fragments/gpio.html");

    EXPECT_NE(fragment.find("name=\"filter\" value=\"all\""), std::string::npos);
    EXPECT_NE(fragment.find("name=\"filter\" value=\"active\""), std::string::npos);
    EXPECT_NE(fragment.find("name=\"filter\" value=\"free\""), std::string::npos);
}

TEST(GpioDashboardAssetsTests, PreservesCollapsibleChipStateAcrossFragmentRefreshes) {
    const auto fragment = readSource(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/templates/fragments/gpio.html");
    const auto runtime = drogular::interactions_resources::script();

    EXPECT_NE(fragment.find("data-dg-preserve-key"), std::string::npos);
    EXPECT_NE(runtime.find("preservedOpenState"), std::string::npos);
    EXPECT_NE(runtime.find("restoreOpenState"), std::string::npos);
}

TEST(GpioDashboardAssetsTests, HidesUnavailableInventoryDeclaratively) {
    const auto fragment = readSource(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/templates/fragments/gpio.html");
    const auto runtime = drogular::interactions_resources::script();
    const auto dashboard = readSource(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/templates/dashboard.html");

    EXPECT_NE(fragment.find("data-dg-unavailable"), std::string::npos);
    EXPECT_NE(runtime.find("element.hasAttribute('dg-hide-on-unavailable')"), std::string::npos);
    EXPECT_NE(dashboard.find("dg-hide-on-unavailable"), std::string::npos);
}