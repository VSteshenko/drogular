#include "uart_status_action.hpp"

#include "hardware/board_gpio_metadata.hpp"
#include "hardware/uart_gpio_correlator.hpp"
#include "services/gpio_service.hpp"
#include "services/uart_service.hpp"
#include "services/system_monitor.hpp"

#include <json/json.h>

#include <chrono>

namespace system_monitor {

namespace {

Json::Value toJson(const UartSnapshot& snapshot,
                   const UartServiceStatistics& statistics,
                   const GpioSnapshot* gpioSnapshot,
                   const BoardGpioMetadata& boardMetadata
) {
    Json::Value root(Json::objectValue);

    const auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch());
    root["timestamp"] = Json::Int64(now.count());
    root["available"] = statistics.available;

    Json::Value devices(Json::arrayValue);
    for (const auto& source : snapshot.devices) {
        Json::Value device(Json::objectValue);
        device["name"] = source.name;
        device["path"] = source.path;
        Json::Value aliases(Json::arrayValue);
        for (const auto& alias : source.aliases) {
            aliases.append(alias);
        }
        device["aliases"] = std::move(aliases);
        devices.append(std::move(device));
    }
    root["devices"] = std::move(devices);

    Json::Value gpioGroups(Json::arrayValue);
    if (gpioSnapshot) {
        for (const auto& sourceGroup : UartGpioCorrelator::groups(*gpioSnapshot, boardMetadata)) {
            Json::Value group(Json::objectValue);
            group["controller"] = sourceGroup.controller;
            group["exposure"] = std::string(gpioExposureName(sourceGroup.exposure));
            Json::Value pins(Json::arrayValue);
            for (const auto& sourcePin : sourceGroup.pins) {
                Json::Value pin(Json::objectValue);
                pin["role"] = sourcePin.role;
                pin["chip"] = sourcePin.chip;
                pin["offset"] = sourcePin.offset;
                pin["name"] = sourcePin.name;
                pin["function"] = sourcePin.function;
                pin["consumer"] = sourcePin.consumer;
                pin["exposure"] = std::string(gpioExposureName(sourcePin.exposure));
                if (sourcePin.physicalHeaderPin) {
                    pin["physicalHeaderPin"] = *sourcePin.physicalHeaderPin;
                } else {
                    pin["physicalHeaderPin"] = Json::nullValue;
                }
                pins.append(std::move(pin));
            }
            group["pins"] = std::move(pins);
            gpioGroups.append(std::move(group));
        }
    }
    root["gpioGroups"] = std::move(gpioGroups);

    Json::Value monitor(Json::objectValue);
    monitor["updates"] = Json::UInt64(statistics.updates);
    monitor["cacheHits"] = Json::UInt64(statistics.cacheHits);
    monitor["failedUpdates"] = Json::UInt64(statistics.failedUpdates);
    monitor["lastUpdateDurationMs"] = Json::Int64(statistics.lastUpdateDuration.count());
    monitor["snapshotAgeMs"] = Json::Int64(statistics.snapshotAge.count());
    monitor["refreshIntervalMs"] = Json::Int64(statistics.refreshInterval.count());
    monitor["healthy"] = statistics.healthy;
    if (statistics.lastSuccessfulUpdate.time_since_epoch() != std::chrono::system_clock::duration::zero()) {
        monitor["lastSuccessfulUpdate"] = Json::Int64(
            std::chrono::duration_cast<std::chrono::seconds>(
                statistics.lastSuccessfulUpdate.time_since_epoch()).count());
    } else {
        monitor["lastSuccessfulUpdate"] = Json::nullValue;
    }
    root["monitor"] = std::move(monitor);

    return root;
}

} // namespace

 drogular::ActionResult UartStatusAction::handle(drogular::ActionContext& context) {
    const auto service = context.requireService<UartService>();
    const auto snapshot = service->snapshot();

    BoardGpioMetadata boardMetadata;
    if (const auto monitor = context.service<SystemMonitor>()) {
        try {
            boardMetadata = BoardGpioMetadata::fromSystemSnapshot(monitor->snapshot());
        } catch (...) {
            // Board metadata is optional enrichment. UART inventory must stay available.
        }
    }

    GpioSnapshot gpioSnapshot;
    const GpioSnapshot* gpioSnapshotPtr = nullptr;
    if (const auto gpioService = context.service<GpioService>();
        gpioService && gpioService->available()
    ) {
        try {
            gpioSnapshot = gpioService->snapshot();
            gpioSnapshotPtr = &gpioSnapshot;
        } catch (...) {
            // GPIO enrichment is optional. UART inventory must stay available.
        }
    }

    return drogular::ActionResult::json(
        toJson(snapshot,
            service->statistics(),
            gpioSnapshotPtr,
            boardMetadata
        ));
}

} // namespace system_monitor