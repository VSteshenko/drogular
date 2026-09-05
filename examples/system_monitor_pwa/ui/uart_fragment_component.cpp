#include "uart_fragment_component.hpp"
#include "hardware/uart_gpio_correlator.hpp"
#include "services/uart_service.hpp"
#include "ui/hardware_fragment_support.hpp"

#include <drogular/render_context.hpp>

#include <json/json.h>

#include <cctype>
#include <algorithm>

namespace system_monitor {

void UartFragmentComponent::onInit(drogular::RenderContext& context) {
    const auto service = context.requireService<UartService>();
    const auto snapshot=service->snapshot();
    const auto stats = service->statistics();
    const auto board = hardware_fragment_support::boardMetadata(context);
    const auto gpio = hardware_fragment_support::gpioSnapshot(context);

    Json::Value devices(Json::arrayValue);
    for (const auto& d: snapshot.devices) {
        Json::Value v;
        v["name"]  = !d.path.empty()
            ? d.path
            : (!d.name.empty() ? d.name : "UART device");

        Json::Value aliases(Json::arrayValue);
        for (const auto& a: d.aliases) {
            aliases.append(a);
        }
        v["aliases"] = std::move(aliases);

        v["hasAliases"]=!d.aliases.empty();

        devices.append(std::move(v));
    }

    Json::Value groups(Json::arrayValue);
    if (gpio) {
        for (const auto& g: UartGpioCorrelator::groups(*gpio,board)) {
            Json::Value v;
            v["name"] = "UART" + std::to_string(g.controller) + " GPIO";
            v["exposureClass"] = "uart-exposure-" + std::string(gpioExposureName(g.exposure));
            v["exposure"] = hardware_fragment_support::exposure(g.exposure);

            Json::Value pins(Json::arrayValue);
            for (const auto& p: g.pins) {
                Json::Value x;
                std::string role = p.role;
                std::transform(
                    role.begin(),
                    role.end(),
                    role.begin(),
                    [](unsigned char c) {
                        return static_cast<char>(std::toupper(c));
                    });
                auto name = p.name.empty()
                    ? p.chip + ":" + std::to_string(p.offset)
                    : p.name;
                x["text"] = role + " · " +
                    name + (p.physicalHeaderPin
                        ? " · pin " + std::to_string(*p.physicalHeaderPin)
                        : ""
                    );
                x["title"] = (p.function.empty() ? p.consumer : p.function) + " · " +
                    hardware_fragment_support::exposure(p.exposure, p.physicalHeaderPin);
                pins.append(std::move(x));
            }
            v["pins"] = std::move(pins);

            groups.append(std::move(v));
        }
    }

    const auto groupCount = groups.size();
    context.set("uartAvailable", stats.available);
    context.set("hasUartDevices", !snapshot.devices.empty());
    context.set("uartDevices", std::move(devices));
    context.set("hasUartGroups", groupCount != 0);
    context.set("uartGroups", std::move(groups));
    context.set("uartSummary", std::to_string(snapshot.devices.size()) + " " +
        context.translate(snapshot.devices.size() == 1 ? "client.device":"client.devices") + " · " +
        std::to_string(groupCount) + " GPIO " +
        context.translate(groupCount == 1 ? "client.group":"client.groups")
    );

    if (!stats.healthy && stats.available) {
        context.set("uartStatus", context.translate("status.stale") + " · " +
            hardware_fragment_support::age(context,stats.snapshotAge) + " " +
            context.translate("client.old")
        );
    } else {
        auto t = hardware_fragment_support::clock(stats.lastSuccessfulUpdate);
        context.set("uartStatus", t.empty()
            ? "UART " + context.translate("client.inventory_available")
            : context.translate("client.updated") + " " + t
        );
    }
}

} // namespace system_monitor