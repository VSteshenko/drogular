#include "i2c_status_action.hpp"
#include "hardware/board_gpio_metadata.hpp"
#include "hardware/i2c_gpio_correlator.hpp"
#include "services/gpio_service.hpp"
#include "services/i2c_service.hpp"
#include "services/system_monitor.hpp"

#include <json/json.h>

#include <chrono>
#include <iomanip>
#include <sstream>

namespace system_monitor {

namespace {

std::string addressText(std::uint8_t address) {
    std::ostringstream stream;
    stream << "0x"
           << std::hex << std::nouppercase
           << std::setw(2) << std::setfill('0')
           << static_cast<unsigned int>(address);
    return stream.str();
}

Json::Value toJson(
    const I2cSnapshot& snapshot,
    const I2cServiceStatistics& statistics,
    const GpioSnapshot* gpioSnapshot,
    const BoardGpioMetadata& boardMetadata
) {
    Json::Value root(Json::objectValue);
    const auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch());
    root["timestamp"] = Json::Int64(now.count());
    root["available"] = statistics.available;

    Json::Value buses(Json::arrayValue);
    for (const auto& source : snapshot.buses) {
        Json::Value bus(Json::objectValue);
        bus["number"] = source.bus.number;
        bus["name"] = source.bus.name;
        bus["type"] = source.bus.type;
        bus["description"] = source.bus.description;
        bus["algorithm"] = source.bus.algorithm;
        bus["scanned"] = source.scanned;

        Json::Value gpioPins(Json::arrayValue);
        GpioExposure busExposure = GpioExposure::Unknown;
        if (gpioSnapshot != nullptr) {
            for (const auto& sourcePin :
                 I2cGpioCorrelator::pinsForBus(
                     source.bus.number, *gpioSnapshot, boardMetadata)) {
                busExposure = combineGpioExposure(busExposure, sourcePin.exposure);

                Json::Value pin(Json::objectValue);
                pin["role"] = sourcePin.role == I2cGpioRole::Sda ? "sda" : "scl";
                pin["chip"] = sourcePin.chip;
                pin["offset"] = sourcePin.offset;
                pin["name"] = sourcePin.name;
                pin["function"] = sourcePin.function;
                pin["exposure"] = std::string(gpioExposureName(sourcePin.exposure));
                if (sourcePin.physicalHeaderPin) {
                    pin["physicalHeaderPin"] = *sourcePin.physicalHeaderPin;
                } else {
                    pin["physicalHeaderPin"] = Json::nullValue;
                }
                gpioPins.append(std::move(pin));
            }
        }
        bus["exposure"] = std::string(gpioExposureName(busExposure));
        bus["gpioPins"] = std::move(gpioPins);

        Json::Value devices(Json::arrayValue);
        for (const auto& sourceDevice : source.devices) {
            Json::Value device(Json::objectValue);
            device["address"] = sourceDevice.address;
            device["addressHex"] = addressText(sourceDevice.address);
            device["claimedByKernel"] = sourceDevice.claimedByKernel;
            devices.append(std::move(device));
        }
        bus["devices"] = std::move(devices);
        buses.append(std::move(bus));
    }
    root["buses"] = std::move(buses);

    Json::Value monitor(Json::objectValue);
    monitor["updates"] = Json::UInt64(statistics.updates);
    monitor["cacheHits"] = Json::UInt64(statistics.cacheHits);
    monitor["failedUpdates"] = Json::UInt64(statistics.failedUpdates);
    monitor["lastUpdateDurationMs"] =
        Json::Int64(statistics.lastUpdateDuration.count());
    monitor["snapshotAgeMs"] = Json::Int64(statistics.snapshotAge.count());
    monitor["refreshIntervalMs"] =
        Json::Int64(statistics.refreshInterval.count());
    monitor["healthy"] = statistics.healthy;

    if (statistics.lastSuccessfulUpdate.time_since_epoch() !=
        std::chrono::system_clock::duration::zero()) {
        const auto lastSuccessfulUpdate =
            std::chrono::duration_cast<std::chrono::seconds>(
                statistics.lastSuccessfulUpdate.time_since_epoch());
        monitor["lastSuccessfulUpdate"] =
            Json::Int64(lastSuccessfulUpdate.count());
    } else {
        monitor["lastSuccessfulUpdate"] = Json::nullValue;
    }

    root["monitor"] = std::move(monitor);
    return root;
}

} // namespace

drogular::ActionResult I2cStatusAction::handle(
    drogular::ActionContext& context
) {
    const auto service = context.requireService<I2cService>();
    const auto snapshot = service->snapshot();

    BoardGpioMetadata boardMetadata;
    if (const auto monitor = context.service<SystemMonitor>()) {
        try {
            boardMetadata = BoardGpioMetadata::fromSystemSnapshot(monitor->snapshot());
        } catch (...) {
            // Board metadata is optional enrichment. I²C inventory must stay available.
        }
    }

    GpioSnapshot gpioSnapshot;
    const GpioSnapshot* gpioSnapshotPtr = nullptr;
    if (const auto gpioService = context.service<GpioService>();
        gpioService && gpioService->available()) {
        try {
            gpioSnapshot = gpioService->snapshot();
            gpioSnapshotPtr = &gpioSnapshot;
        } catch (...) {
            // GPIO correlation is optional enrichment. I²C inventory must
            // remain available even when GPIO probing is temporarily unavailable.
        }
    }

    return drogular::ActionResult::json(
        toJson(snapshot, service->statistics(), gpioSnapshotPtr, boardMetadata));
}

} // namespace system_monitor