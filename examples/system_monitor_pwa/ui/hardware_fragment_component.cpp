#include "hardware_fragment_component.hpp"

#include "hardware/board_gpio_metadata.hpp"
#include "hardware/i2c_gpio_correlator.hpp"
#include "hardware/spi_gpio_correlator.hpp"
#include "hardware/uart_gpio_correlator.hpp"
#include "services/gpio_service.hpp"
#include "services/i2c_service.hpp"
#include "services/spi_service.hpp"
#include "services/system_monitor.hpp"
#include "services/uart_service.hpp"

#include <drogular/render_context.hpp>

#include <json/json.h>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace system_monitor {
namespace {

struct HeaderRole {
    std::string label;
    std::string detail;
};

struct HeaderLine {
    GpioLineInfo line;
    std::string chip;
};

template <typename Service, typename Snapshot>
std::optional<Snapshot> optionalSnapshot(const std::shared_ptr<Service>& service, int& reachable) {
    if (!service) return std::nullopt;
    ++reachable;
    try {
        return service->snapshot();
    } catch (...) {
        return std::nullopt;
    }
}

std::string exposureText(GpioExposure exposure) {
    switch (exposure) {
        case GpioExposure::Header: return "40-pin header";
        case GpioExposure::OnBoard: return "onboard";
        case GpioExposure::Internal: return "internal";
        case GpioExposure::Mixed: return "mixed";
        case GpioExposure::Unknown: return "unknown";
    }
    return "unknown";
}

std::string exposureVariant(GpioExposure exposure) {
    switch (exposure) {
        case GpioExposure::Header: return "dg-badge-success";
        case GpioExposure::Internal: return "dg-badge-info";
        case GpioExposure::OnBoard:
        case GpioExposure::Mixed: return "dg-badge-warning";
        case GpioExposure::Unknown: return "dg-badge-neutral";
    }
    return "dg-badge-neutral";
}

template <typename Statistics>
std::string healthLabel(drogular::RenderContext& context, const Statistics& stats) {
    if (!stats.available) return context.translate("status.unavailable");
    return context.translate(stats.healthy ? "status.healthy" : "status.stale");
}

std::string upper(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return value;
}

std::string addressHex(std::uint8_t address) {
    std::ostringstream stream;
    stream << "0x" << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<unsigned int>(address);
    return stream.str();
}

std::string pinBadge(const std::string& role,
                     const std::string& name,
                     const std::string& function,
                     std::uint32_t offset,
                     std::optional<std::uint32_t> physicalPin,
                     bool hasHeader) {
    const auto label = upper(role);
    if (hasHeader && physicalPin) {
        return label + " · pin " + std::to_string(*physicalPin);
    }
    auto gpioName = name.empty() ? "GPIO" + std::to_string(offset) : name;
    return label + " · " + gpioName +
        (!function.empty() && function != gpioName ? " · " + function : "");
}

Json::Value interfaceCard(std::string title,
                          std::string status,
                          GpioExposure exposure,
                          std::string details,
                          Json::Value badges = Json::Value(Json::arrayValue)) {
    Json::Value card;
    card["title"] = std::move(title);
    card["status"] = std::move(status);
    card["exposure"] = exposureText(exposure);
    card["exposureVariant"] = exposureVariant(exposure);
    card["details"] = std::move(details);
    card["badges"] = std::move(badges);
    card["hasBadges"] = card["badges"].size() != 0;
    return card;
}

std::map<std::uint32_t, HeaderLine> headerLines(const GpioSnapshot& gpio,
                                                const BoardGpioMetadata& board) {
    std::map<std::uint32_t, HeaderLine> result;
    for (const auto& chip : gpio.chips) {
        for (const auto& line : chip.lines) {
            const auto metadata = board.line(chip.chip.name, line.offset);
            if (metadata.physicalHeaderPin) {
                result[*metadata.physicalHeaderPin] = HeaderLine{line, chip.chip.name};
            }
        }
    }
    return result;
}

Json::Value physicalPin(std::uint32_t number,
                        const std::map<std::uint32_t, HeaderLine>& gpioByPin,
                        const std::map<std::uint32_t, std::vector<HeaderRole>>& rolesByPin) {
    static const std::map<std::uint32_t, std::pair<std::string, std::string>> staticPins{
        {1, {"3V3", "power"}}, {2, {"5V", "power"}}, {4, {"5V", "power"}},
        {6, {"GND", "ground"}}, {9, {"GND", "ground"}}, {14, {"GND", "ground"}},
        {17, {"3V3", "power"}}, {20, {"GND", "ground"}}, {25, {"GND", "ground"}},
        {30, {"GND", "ground"}}, {34, {"GND", "ground"}}, {39, {"GND", "ground"}}
    };

    Json::Value pin;
    pin["number"] = number;
    const auto staticIt = staticPins.find(number);
    const auto gpioIt = gpioByPin.find(number);
    const auto rolesIt = rolesByPin.find(number);
    const bool hasRoles = rolesIt != rolesByPin.end() && !rolesIt->second.empty();
    const auto kind = staticIt == staticPins.end() ? "gpio" : staticIt->second.second;
    pin["class"] = "board-pin board-pin-" + kind + (hasRoles ? " board-pin-interface" : "");
    pin["label"] = staticIt != staticPins.end()
        ? staticIt->second.first
        : gpioIt != gpioByPin.end() && !gpioIt->second.line.name.empty()
            ? gpioIt->second.line.name
            : "Pin " + std::to_string(number);

    pin["hasDetail"] = gpioIt != gpioByPin.end();
    if (gpioIt != gpioByPin.end()) {
        const auto& line = gpioIt->second.line;
        const auto state = line.used ? "used" : line.alternateFunction ? "muxed" : "free";
        const auto function = !line.function.empty() ? line.function : "GPIO";
        pin["detail"] = function + " · " + state;
    }

    Json::Value roles(Json::arrayValue);
    if (hasRoles) {
        for (const auto& role : rolesIt->second) {
            Json::Value value;
            value["label"] = role.label;
            value["detail"] = role.detail;
            roles.append(std::move(value));
        }
    }
    pin["roles"] = std::move(roles);
    pin["hasRoles"] = pin["roles"].size() != 0;
    return pin;
}

} // namespace

void HardwareFragmentComponent::onInit(drogular::RenderContext& context) {
    const auto monitor = context.requireService<SystemMonitor>();
    const auto system = monitor->snapshot(); // A failed system snapshot should fail the interaction.
    const auto board = BoardGpioMetadata::fromSystemSnapshot(system);
    const bool hasHeader = board.available();

    int reachable = 1; // system
    const auto gpioService = context.service<GpioService>();
    const auto i2cService = context.service<I2cService>();
    const auto spiService = context.service<SpiService>();
    const auto uartService = context.service<UartService>();

    const auto gpio = optionalSnapshot<GpioService, GpioSnapshot>(gpioService, reachable);
    const auto i2c = optionalSnapshot<I2cService, I2cSnapshot>(i2cService, reachable);
    const auto spi = optionalSnapshot<SpiService, SpiSnapshot>(spiService, reachable);
    const auto uart = optionalSnapshot<UartService, UartSnapshot>(uartService, reachable);

    const auto gpioStats = gpioService ? gpioService->statistics() : GpioServiceStatistics{};
    const auto i2cStats = i2cService ? i2cService->statistics() : I2cServiceStatistics{};
    const auto spiStats = spiService ? spiService->statistics() : SpiServiceStatistics{};
    const auto uartStats = uartService ? uartService->statistics() : UartServiceStatistics{};
    const int availableInventories = static_cast<int>(gpioStats.available) +
        static_cast<int>(i2cStats.available) + static_cast<int>(spiStats.available) +
        static_cast<int>(uartStats.available);

    context.set("hostname", system.system.hostname);
    context.set("hasRaspberryPi", system.raspberryPi.has_value());
    context.set("hasBoardHeader", hasHeader);
    context.set("hardwareIdentityLabel", context.translate(system.raspberryPi
        ? "board.identity_board"
        : "nav.system"));
    context.set("hardwareSubtitle", context.translate(hasHeader
        ? "board.subtitle_pi"
        : "board.subtitle_generic"));
    context.set("boardOverviewClass", std::string(hasHeader
        ? "board-overview-grid"
        : "board-overview-grid board-overview-grid-generic"));

    std::string boardModel = system.system.operatingSystem;
    if (system.raspberryPi) {
        boardModel = system.raspberryPi->model;
        context.set("boardRevision", system.raspberryPi->revision.empty()
            ? context.translate("status.unavailable")
            : system.raspberryPi->revision);
        context.set("boardSerial", system.raspberryPi->serial.empty()
            ? context.translate("status.unavailable")
            : system.raspberryPi->serial);
    } else if (!system.system.architecture.empty()) {
        boardModel += " · " + system.system.architecture;
    }
    context.set("boardModel", boardModel);
    context.set("inventoryStatus", std::to_string(reachable) + "/5 " + context.translate("client.data_sources_reachable") +
        " · " + std::to_string(availableInventories) + "/4 " + context.translate("client.hardware_inventories_available"));

    std::size_t gpioLinesCount = 0;
    std::size_t gpioHeaderCount = 0;
    std::map<std::uint32_t, HeaderLine> gpioByPin;
    if (gpio) {
        for (const auto& chip : gpio->chips) gpioLinesCount += chip.lines.size();
        if (hasHeader) {
            gpioByPin = headerLines(*gpio, board);
            gpioHeaderCount = gpioByPin.size();
        }
    }
    context.set("gpioSummaryValue", gpioStats.available
        ? std::to_string(hasHeader ? gpioHeaderCount : gpioLinesCount) + " " + context.translate(hasHeader ? "client.header_gpio" : "client.gpio_lines")
        : context.translate("status.unavailable"));
    context.set("gpioSummaryDetail", gpioStats.available
        ? std::to_string(gpio ? gpio->chips.size() : 0) + " " + context.translate("client.chips") + " · " +
          std::to_string(gpioLinesCount) + " " + context.translate("client.lines") + " · " + healthLabel(context, gpioStats)
        : "GPIO " + context.translate("client.inventory_unavailable"));

    std::size_t i2cDevices = 0;
    if (i2c) for (const auto& bus : i2c->buses) i2cDevices += bus.devices.size();
    context.set("i2cSummaryValue", i2cStats.available
        ? std::to_string(i2c ? i2c->buses.size() : 0) + " " + context.translate("client.buses")
        : context.translate("status.unavailable"));
    context.set("i2cSummaryDetail", i2cStats.available
        ? std::to_string(i2cDevices) + " " + context.translate("client.detected_devices") + " · " + healthLabel(context, i2cStats)
        : "I²C " + context.translate("client.inventory_unavailable"));

    std::map<std::uint32_t, std::vector<SpiDeviceInfo>> spiBuses;
    if (spi) for (const auto& device : spi->devices) spiBuses[device.bus].push_back(device);
    context.set("spiSummaryValue", spiStats.available
        ? std::to_string(spiBuses.size()) + " " + context.translate("client.buses")
        : context.translate("status.unavailable"));
    context.set("spiSummaryDetail", spiStats.available
        ? std::to_string(spi ? spi->devices.size() : 0) + " " + context.translate("client.spidev_nodes") + " · " + healthLabel(context, spiStats)
        : "SPI " + context.translate("client.inventory_unavailable"));

    const auto uartGroups = gpio ? UartGpioCorrelator::groups(*gpio, board) : std::vector<UartGpioGroup>{};
    context.set("uartSummaryValue", uartStats.available
        ? std::to_string(uart ? uart->devices.size() : 0) + " " + context.translate("client.devices")
        : context.translate("status.unavailable"));
    context.set("uartSummaryDetail", uartStats.available
        ? std::to_string(uartGroups.size()) + " " + context.translate("client.pinmux_groups") + " · " + healthLabel(context, uartStats)
        : "UART " + context.translate("client.inventory_unavailable"));

    std::map<std::uint32_t, std::vector<HeaderRole>> rolesByPin;
    auto addRole = [&](std::optional<std::uint32_t> physicalPin, std::string label, std::string detail) {
        if (physicalPin) rolesByPin[*physicalPin].push_back(HeaderRole{std::move(label), std::move(detail)});
    };

    if (gpio && i2c) {
        for (const auto& bus : i2c->buses) {
            for (const auto& pin : I2cGpioCorrelator::pinsForBus(bus.bus.number, *gpio, board)) {
                const auto role = pin.role == I2cGpioRole::Sda ? "SDA" : "SCL";
                addRole(pin.physicalHeaderPin, "I²C" + std::to_string(bus.bus.number) + " " + role, pin.function);
            }
        }
    }
    if (gpio) {
        for (const auto& [number, devices] : spiBuses) {
            for (const auto& pin : SpiGpioCorrelator::pinsForBus(number, *gpio, board)) {
                addRole(pin.physicalHeaderPin, "SPI" + std::to_string(number) + " " + upper(pin.role),
                        pin.function.empty() ? pin.consumer : pin.function);
            }
        }
        for (const auto& group : uartGroups) {
            for (const auto& pin : group.pins) {
                addRole(pin.physicalHeaderPin, "UART" + std::to_string(group.controller) + " " + upper(pin.role), pin.function);
            }
        }
    }

    Json::Value headerRows(Json::arrayValue);
    if (hasHeader) {
        for (std::uint32_t row = 0; row < 20; ++row) {
            Json::Value value;
            const auto left = row * 2 + 1;
            value["left"] = physicalPin(left, gpioByPin, rolesByPin);
            value["right"] = physicalPin(left + 1, gpioByPin, rolesByPin);
            headerRows.append(std::move(value));
        }
    }
    context.set("headerRows", std::move(headerRows));
    context.set("hasHeaderMetadata", hasHeader && !gpioByPin.empty());
    context.set("headerStatus", hasHeader && !gpioByPin.empty()
        ? std::to_string(gpioHeaderCount) + " GPIO pins · " + std::to_string(rolesByPin.size()) + " interface pins"
        : context.translate("client.header_metadata_unavailable"));

    Json::Value cards(Json::arrayValue);
    Json::Value gpioBadges(Json::arrayValue);
    if (gpio) {
        std::size_t active = 0;
        for (const auto& chip : gpio->chips) {
            for (const auto& line : chip.lines) {
                const auto metadata = board.line(chip.chip.name, line.offset);
                if (hasHeader && !metadata.physicalHeaderPin) continue;
                if (!line.used && !line.alternateFunction) continue;
                ++active;
                if (gpioBadges.size() < 8) {
                    gpioBadges.append(hasHeader && metadata.physicalHeaderPin
                        ? (line.name.empty() ? "GPIO" + std::to_string(line.offset) : line.name) + " · pin " + std::to_string(*metadata.physicalHeaderPin)
                        : (line.name.empty() ? "GPIO" + std::to_string(line.offset) : line.name) + (line.function.empty() ? "" : " · " + line.function));
                }
            }
        }
        cards.append(interfaceCard("GPIO", healthLabel(context, gpioStats),
            hasHeader && gpioHeaderCount > 0 ? GpioExposure::Header : GpioExposure::Unknown,
            std::to_string(active) + " active or muxed " + (hasHeader ? "header " : "") + "lines",
            std::move(gpioBadges)));
    } else {
        cards.append(interfaceCard("GPIO", context.translate("status.unavailable"), GpioExposure::Unknown,
            context.translate("client.gpio_service_unavailable")));
    }

    if (i2c && gpio) {
        for (const auto& bus : i2c->buses) {
            auto pins = I2cGpioCorrelator::pinsForBus(bus.bus.number, *gpio, board);
            GpioExposure exposure = GpioExposure::Unknown;
            Json::Value badges(Json::arrayValue);
            for (const auto& pin : pins) {
                exposure = combineGpioExposure(exposure, pin.exposure);
                badges.append(pinBadge(pin.role == I2cGpioRole::Sda ? "sda" : "scl", pin.name, pin.function,
                                       pin.offset, pin.physicalHeaderPin, hasHeader));
            }
            for (const auto& device : bus.devices) badges.append(addressHex(device.address));
            const auto status = bus.scanned
                ? std::to_string(bus.devices.size()) + " detected " + context.translate(bus.devices.size() == 1 ? "client.device" : "client.devices")
                : "Inventory only";
            cards.append(interfaceCard("I²C " + std::to_string(bus.bus.number), status, exposure,
                !bus.bus.description.empty() ? bus.bus.description : !bus.bus.name.empty() ? bus.bus.name : "I²C bus",
                std::move(badges)));
        }
    }

    if (gpio) {
        for (const auto& [number, devices] : spiBuses) {
            auto pins = SpiGpioCorrelator::pinsForBus(number, *gpio, board);
            GpioExposure exposure = GpioExposure::Unknown;
            Json::Value badges(Json::arrayValue);
            for (const auto& pin : pins) {
                exposure = combineGpioExposure(exposure, pin.exposure);
                badges.append(pinBadge(pin.role, pin.name, pin.function, pin.offset, pin.physicalHeaderPin, hasHeader));
            }
            std::string details;
            for (const auto& device : devices) {
                if (!details.empty()) details += " · ";
                details += device.path;
            }
            if (details.empty()) details = context.translate("client.no_spi");
            cards.append(interfaceCard("SPI " + std::to_string(number),
                std::to_string(devices.size()) + " " + (devices.size() == 1 ? "device node" : "device nodes"),
                exposure, details, std::move(badges)));
        }
    }

    std::vector<std::string> uartAliases;
    if (uart) {
        for (const auto& device : uart->devices) {
            uartAliases.push_back(device.path.empty() ? device.name : device.path);
            uartAliases.insert(uartAliases.end(), device.aliases.begin(), device.aliases.end());
        }
    }
    for (const auto& group : uartGroups) {
        Json::Value badges(Json::arrayValue);
        for (const auto& pin : group.pins) {
            badges.append(pinBadge(pin.role, pin.name, pin.function, pin.offset, pin.physicalHeaderPin, hasHeader));
        }
        std::string details;
        for (const auto& alias : uartAliases) {
            if (!details.empty()) details += " · ";
            details += alias;
        }
        details = details.empty() ? context.translate("client.no_linux_serial") : "Linux serial: " + details;
        cards.append(interfaceCard("UART " + std::to_string(group.controller),
            context.translate(group.exposure == GpioExposure::Header ? "client.available_on_header" : "client.detected_pinmux"),
            group.exposure, details, std::move(badges)));
    }
    context.set("interfaceCards", std::move(cards));
}

} // namespace system_monitor