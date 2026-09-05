#include "spi_fragment_component.hpp"
#include "hardware/spi_gpio_correlator.hpp"
#include "services/spi_service.hpp"
#include "ui/hardware_fragment_support.hpp"

#include <drogular/render_context.hpp>

#include <json/json.h>

#include <algorithm>
#include <cctype>
#include <map>

namespace system_monitor {

void SpiFragmentComponent::onInit(drogular::RenderContext& context) {
    const auto service = context.requireService<SpiService>();
    const auto snapshot=service->snapshot();
    const auto stats = service->statistics();
    const auto board = hardware_fragment_support::boardMetadata(context);
    const auto gpio = hardware_fragment_support::gpioSnapshot(context);

    std::map<std::uint32_t,std::vector<SpiDeviceInfo>> grouped;
    for (const auto& d: snapshot.devices) {
        grouped[d.bus].push_back(d);
    }

    Json::Value buses(Json::arrayValue);
    for (const auto& [number, devices] : grouped) {
        Json::Value bus;
        bus["name"] = "spi" + std::to_string(number);
        bus["description"] = "Linux spidev bus";
        bus["deviceCountText"] = std::to_string(devices.size()) + " " +
            context.translate(devices.size() == 1 ? "client.device" : "client.devices");

        Json::Value pins(Json::arrayValue);
        if (gpio) {
            for(const auto& pin:SpiGpioCorrelator::pinsForBus(number, *gpio, board)) {
                Json::Value v;
                auto name = pin.name.empty()
                    ? pin.chip + ":" + std::to_string(pin.offset)
                    : pin.name;
                std::string role = pin.role;
                std::transform(
                    role.begin(),
                    role.end(),
                    role.begin(),
                    [](unsigned char c) {
                        return static_cast<char>(std::toupper(c));
                    });
                v["text"] = role + " · " + name + (pin.physicalHeaderPin
                    ?" · pin " + std::to_string(*pin.physicalHeaderPin)
                    : "");
                v["title"] = (pin.consumer.empty() ? pin.function : pin.consumer) + " · " +
                    hardware_fragment_support::exposure(pin.exposure, pin.physicalHeaderPin);
                pins.append(std::move(v));
            }
        }
        bus["pins"] = std::move(pins);

        bus["hasPins"] = bus["pins"].size() != 0;

        Json::Value items(Json::arrayValue);
        for (const auto& d: devices) {
            Json::Value v;
            v["path"] = d.path.empty()
                ? "/dev/spidev" + std::to_string(number) + "." +
                    std::to_string(d.chipSelect)
                : d.path;
            v["title"] = "Chip select " + std::to_string(d.chipSelect);
            items.append(std::move(v));
        }
        bus["devices"]=std::move(items);

        buses.append(std::move(bus));
    }

    context.set("spiAvailable", stats.available);
    context.set("hasSpiBuses", !grouped.empty());
    context.set("spiBuses", std::move(buses));
    context.set("spiSummary", std::to_string(grouped.size()) + " " +
        context.translate("client.buses") + " · " +
        std::to_string(snapshot.devices.size()) + " " +
        context.translate(snapshot.devices.size() == 1 ? "client.device":"client.devices")
    );

    if (!stats.healthy && stats.available) {
        context.set("spiStatus", context.translate("status.stale") + " · " +
            hardware_fragment_support::age(context,stats.snapshotAge) + " " +
            context.translate("client.old")
        );
    } else {
        auto t = hardware_fragment_support::clock(stats.lastSuccessfulUpdate);
        context.set("spiStatus", t.empty()
            ? "SPI " + context.translate("client.inventory_available")
            : context.translate("client.updated") + " " + t);
    }
}

}