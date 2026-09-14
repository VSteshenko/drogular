#include <drogular/ui_resources.hpp>

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

TEST(HardwareBoardAssetsTests, UsesServerRenderedHardwareFragmentAndBuiltInUi) {
    const auto root = std::filesystem::path(DROGULAR_SOURCE_DIR) /
                      "examples/system_monitor_pwa";
    const auto page = readAsset(root / "templates/board.html");
    const auto fragment = readAsset(root / "templates/fragments/hardware.html");
    const auto css = readAsset(root / "public/app.css");
    const auto worker = readAsset(root / "public/service-worker.js");
    const auto ui = drogular::ui_resources::stylesheet();

    EXPECT_NE(page.find("dg-get=\"/fragments/system\""), std::string::npos);
    EXPECT_NE(page.find("dg-trigger=\"every 2s\""), std::string::npos);
    EXPECT_NE(page.find("data-board-connection"), std::string::npos);
    EXPECT_NE(page.find("class=\"hero board-hero\""), std::string::npos);
    EXPECT_NE(page.find("dg-get=\"/fragments/hardware\""), std::string::npos);
    EXPECT_NE(page.find("dg-trigger=\"every 30s\""), std::string::npos);
    EXPECT_NE(page.find("data-board-fragment-host"), std::string::npos);
    EXPECT_EQ(page.find("/assets/board.js"), std::string::npos);

    EXPECT_EQ(fragment.find("class=\"hero board-hero\""), std::string::npos);
    EXPECT_NE(fragment.find("data-board-header-map"), std::string::npos);
    EXPECT_NE(fragment.find("data-board-interface-list"), std::string::npos);
    EXPECT_NE(fragment.find("@foreach(row in headerRows)"), std::string::npos);
    EXPECT_NE(fragment.find("@foreach(card in interfaceCards)"), std::string::npos);
    EXPECT_NE(fragment.find("panel dg-card board-interface-card"), std::string::npos);

    EXPECT_EQ(worker.find("/assets/board.js"), std::string::npos);
    EXPECT_NE(worker.find("drogular-system-monitor-v24"), std::string::npos);

    EXPECT_NE(css.find(".board-header-map"), std::string::npos);
    EXPECT_NE(css.find(".hardware-summary-grid"), std::string::npos);
    EXPECT_NE(css.find(".board-interface-card"), std::string::npos);
    EXPECT_NE(css.find(".board-overview-grid-generic"), std::string::npos);
    EXPECT_NE(ui.find(".dg-card"), std::string::npos);
    EXPECT_NE(ui.find(".dg-button"), std::string::npos);
    EXPECT_NE(ui.find(".dg-badge"), std::string::npos);
    EXPECT_NE(css.find("[hidden] { display: none !important; }"), std::string::npos);
}