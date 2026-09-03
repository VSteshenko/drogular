#include "board_page.hpp"

#include "hardware/board_gpio_metadata.hpp"
#include "services/system_monitor.hpp"

#include <drogular/pwa_page_support.hpp>
#include <drogular/render_context.hpp>

#include <stdexcept>
#include <string>

namespace system_monitor {

void BoardPage::onInit(drogular::RenderContext& context) {
    drogular::PwaOptions pwaOptions;
    pwaOptions.themeColor = "#10172a";
    drogular::PwaPageSupport::apply(context, pwaOptions);
    const auto monitor = context.service<SystemMonitor>();
    if (monitor == nullptr) {
        throw std::runtime_error("SystemMonitor service is not registered");
    }

    const auto snapshot = monitor->snapshot();

    context.set("title", std::string("Drogular Hardware Overview"));
    context.set("pageScript", std::string("/assets/board.js"));
    context.set("hasPageScript", true);
    context.set("hostname", snapshot.system.hostname);
    const auto boardMetadata = BoardGpioMetadata::fromSystemSnapshot(snapshot);
    const bool hasRaspberryPi = snapshot.raspberryPi.has_value();

    context.set("hasRaspberryPi", hasRaspberryPi);
    context.set("hasBoardHeader", boardMetadata.available());
    context.set("hardwareIdentityLabel", std::string(hasRaspberryPi ? "Board" : "System"));
    context.set("hardwareSubtitle", std::string(
        boardMetadata.available()
            ? "board interfaces and physical header"
            : "hardware interfaces detected on this monitoring target"));
    context.set("boardOverviewClass", std::string(
        boardMetadata.available()
            ? "board-overview-grid"
            : "board-overview-grid board-overview-grid-generic"));

    if (snapshot.raspberryPi) {
        context.set("boardModel", snapshot.raspberryPi->model);
        context.set("boardRevision", snapshot.raspberryPi->revision);
        context.set("boardSerial", snapshot.raspberryPi->serial);
    } else {
        std::string systemIdentity = snapshot.system.operatingSystem;
        if (!snapshot.system.architecture.empty()) {
            systemIdentity += " · " + snapshot.system.architecture;
        }
        context.set("boardModel", systemIdentity);
        context.set("boardRevision", std::string("Unavailable"));
        context.set("boardSerial", std::string("Unavailable"));
    }
}

} // namespace system_monitor