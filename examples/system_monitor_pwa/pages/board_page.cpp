#include "board_page.hpp"

#include "services/system_monitor.hpp"

#include <drogular/render_context.hpp>

#include <stdexcept>
#include <string>

namespace system_monitor {

void BoardPage::onInit(drogular::RenderContext& context) {
    const auto monitor = context.service<SystemMonitor>();
    if (monitor == nullptr) {
        throw std::runtime_error("SystemMonitor service is not registered");
    }

    const auto snapshot = monitor->snapshot();

    context.set("title", std::string("Drogular Hardware Overview"));
    context.set("pageScript", std::string("/assets/board.js"));
    context.set("hostname", snapshot.system.hostname);
    context.set("hasRaspberryPi", snapshot.raspberryPi.has_value());

    if (snapshot.raspberryPi) {
        context.set("boardModel", snapshot.raspberryPi->model);
        context.set("boardRevision", snapshot.raspberryPi->revision);
        context.set("boardSerial", snapshot.raspberryPi->serial);
    } else {
        context.set("boardModel", snapshot.system.hostname);
        context.set("boardRevision", std::string("Unavailable"));
        context.set("boardSerial", std::string("Unavailable"));
    }
}

} // namespace system_monitor