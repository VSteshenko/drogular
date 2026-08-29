#include "i2c_status_action.hpp"
#include "services/i2c_service.hpp"

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
    const I2cServiceStatistics& statistics
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
    return drogular::ActionResult::json(toJson(snapshot, service->statistics()));
}

} // namespace system_monitor