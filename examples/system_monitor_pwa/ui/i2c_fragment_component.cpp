#include "i2c_fragment_component.hpp"
#include "hardware/i2c_gpio_correlator.hpp"
#include "services/i2c_service.hpp"
#include "ui/hardware_fragment_support.hpp"

#include <drogular/render_context.hpp>

#include <json/json.h>

#include <iomanip>
#include <sstream>

namespace system_monitor {

namespace {

std::string addressText(std::uint8_t address) {
    std::ostringstream stream;
    stream << "0x" << std::hex <<
        std::nouppercase << std::setw(2) <<
        std::setfill('0') << static_cast<unsigned>(address);

    return stream.str();
}

} // namespace

void I2cFragmentComponent::onInit(drogular::RenderContext& context) {
    const auto service = context.requireService<I2cService>();
    const auto snapshot = service->snapshot();
    const auto stats = service->statistics();
    const auto board = hardware_fragment_support::boardMetadata(context);
    const auto gpio = hardware_fragment_support::gpioSnapshot(context);

    Json::Value buses(Json::arrayValue); std::size_t deviceCount=0;
    for (const auto& source: snapshot.buses) {
        Json::Value bus(Json::objectValue);
        bus["name"] = source.bus.name.empty()
            ? "i2c-" + std::to_string(source.bus.number)
            : source.bus.name;
        bus["description"] = source.bus.description.empty()
            ? "I²C adapter"
            : source.bus.description;

        std::string meta;
        if (!source.bus.type.empty()) {
            meta=source.bus.type;
        }
        if (!source.bus.algorithm.empty()) {
            if (!meta.empty()) {
                meta += " · ";
            }
            meta += source.bus.algorithm;
        }

        bus["metadata"] = meta.empty()
            ? context.translate("client.adapter_unavailable")
            : meta;
        bus["scanned"] = source.scanned;
        bus["deviceCount"] = static_cast<Json::UInt64>(source.devices.size());
        bus["deviceCountText"] = source.scanned
            ? std::to_string(source.devices.size()) + " " +
                context.translate(source.devices.size() == 1 ? "client.device" : "client.devices")
            : context.translate("client.not_scanned");

        Json::Value pins(Json::arrayValue);
        if (gpio) {
            for (const auto& pin:I2cGpioCorrelator::pinsForBus(source.bus.number, *gpio, board)) {
                Json::Value v;
                const auto role = pin.role == I2cGpioRole::Sda ? "SDA" : "SCL";
                const auto name = pin.name.empty()
                    ? pin.chip + ":" + std::to_string(pin.offset)
                    : pin.name;
                v["text"] = std::string(role) + " · " + name +
                    (pin.physicalHeaderPin
                        ? " · pin " + std::to_string(*pin.physicalHeaderPin)
                        : ""
                    );
                v["title"] = (pin.function.empty() ? pin.chip : pin.function) + " · " +
                    hardware_fragment_support::exposure(pin.exposure, pin.physicalHeaderPin);
                pins.append(std::move(v));
            }
        }
        bus["pins"] = std::move(pins);
        bus["hasPins"] = bus["pins"].size() != 0;

        Json::Value devices(Json::arrayValue);
        for (const auto& d: source.devices) {
            Json::Value v;
            v["address"] = addressText(d.address);
            v["class"] = d.claimedByKernel
                ? "i2c-address i2c-address-claimed"
                : "i2c-address";
            v["title"] = d.claimedByKernel
                ? context.translate("client.kernel_driver")
                : "";
            devices.append(std::move(v));
        }

        deviceCount += source.devices.size();
        bus["devices"] = std::move(devices);
        bus["hasDevices"] = source.devices.size() != 0;
        buses.append(std::move(bus));
    }

    context.set("i2cAvailable", stats.available);
    context.set("hasI2cBuses", !snapshot.buses.empty());
    context.set("i2cBuses", std::move(buses));
    context.set("i2cSummary", std::to_string(snapshot.buses.size()) + " " +
        context.translate("client.buses") + " · " + std::to_string(deviceCount) + " " +
        context.translate(deviceCount == 1 ? "client.device":"client.devices"));
    if (!stats.healthy && stats.available) {
        context.set("i2cStatus", context.translate("status.stale") + " · " +
            hardware_fragment_support::age(context, stats.snapshotAge) + " " +
            context.translate("client.old"));
    } else {
        auto t = hardware_fragment_support::clock(stats.lastSuccessfulUpdate);
        context.set("i2cStatus", t.empty()
            ? "I²C " + context.translate("client.inventory_available")
            : context.translate("client.updated") + " " + t);
    }
}

} // namespace system_monitor
