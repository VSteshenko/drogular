#include "spi_status_action.hpp"
#include "hardware/spi_gpio_correlator.hpp"
#include "services/gpio_service.hpp"
#include "services/spi_service.hpp"

#include <json/json.h>

#include <chrono>
#include <map>

namespace system_monitor {

namespace {

Json::Value toJson(const SpiSnapshot& snapshot,
    const SpiServiceStatistics& statistics,
    const GpioSnapshot* gpioSnapshot
) {
    Json::Value root(Json::objectValue);

    const auto now =
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        );
    root["timestamp"] = Json::Int64(now.count());
    root["available"] = statistics.available;

    std::map<std::uint32_t, Json::Value> buses;
    for (const auto& device : snapshot.devices) {
        if (!buses.contains(device.bus)) {
            Json::Value bus(Json::objectValue);

            bus["number"] = device.bus;
            bus["gpioPins"] = Json::arrayValue;
            bus["devices"] = Json::arrayValue;

            if (gpioSnapshot) {
                for (const auto& sourcePin : SpiGpioCorrelator::pinsForBus(device.bus, *gpioSnapshot)) {
                    Json::Value pin(Json::objectValue);

                    pin["role"] = sourcePin.role;
                    pin["chip"] = sourcePin.chip;
                    pin["offset"] = sourcePin.offset;
                    pin["name"] = sourcePin.name;
                    pin["function"] = sourcePin.function;
                    pin["consumer"] = sourcePin.consumer;
                    bus["gpioPins"].append(std::move(pin));
                }
            }
            buses.emplace(device.bus, std::move(bus));
        }

        Json::Value item(Json::objectValue);

        item["chipSelect"] = device.chipSelect;
        item["path"] = device.path;
        buses.at(device.bus)["devices"].append(std::move(item));
    }

    Json::Value jsonBuses(Json::arrayValue);

    for (auto& [number, bus] : buses) {
        jsonBuses.append(std::move(bus));
    }
    root["buses"] = std::move(jsonBuses);

    Json::Value monitor(Json::objectValue);

    monitor["updates"] = Json::UInt64(statistics.updates);
    monitor["cacheHits"] = Json::UInt64(statistics.cacheHits);
    monitor["failedUpdates"] = Json::UInt64(statistics.failedUpdates);
    monitor["lastUpdateDurationMs"] = Json::Int64(statistics.lastUpdateDuration.count());
    monitor["snapshotAgeMs"] = Json::Int64(statistics.snapshotAge.count());
    monitor["refreshIntervalMs"] = Json::Int64(statistics.refreshInterval.count());
    monitor["healthy"] = statistics.healthy;

    if (statistics.lastSuccessfulUpdate.time_since_epoch() != std::chrono::system_clock::duration::zero()) {
        monitor["lastSuccessfulUpdate"] =
            Json::Int64(std::chrono::duration_cast<std::chrono::seconds>(
                statistics.lastSuccessfulUpdate.time_since_epoch()).count()
            );
    } else {
        monitor["lastSuccessfulUpdate"] = Json::nullValue;
    }

    root["monitor"] = std::move(monitor);

    return root;
}

} // namespace

drogular::ActionResult SpiStatusAction::handle(drogular::ActionContext& context) {
    const auto service = context.requireService<SpiService>();
    const auto snapshot = service->snapshot();
    GpioSnapshot gpioSnapshot;
    const GpioSnapshot* gpioSnapshotPtr = nullptr;

    if (const auto gpioService = context.service<GpioService>();
        gpioService && gpioService->available()
    ) {
        try {
            gpioSnapshot = gpioService->snapshot();
            gpioSnapshotPtr = &gpioSnapshot;
        } catch (...) {
        }
    }

    return drogular::ActionResult::json(toJson(snapshot, service->statistics(), gpioSnapshotPtr));
}

} // namespace system_monitor