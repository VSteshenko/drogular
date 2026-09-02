#include "process_status_action.hpp"
#include "services/process_service.hpp"

#include <json/json.h>

#include <chrono>
#include <utility>

namespace system_monitor {

namespace {

Json::Value toJson(
    const std::vector<ProcessInfo>& processes,
    const ProcessStatistics& statistics)
{
    Json::Value root(Json::objectValue);
    const auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch());
    root["timestamp"] = Json::Int64(now.count());

    Json::Value items(Json::arrayValue);
    for (const auto& source : processes) {
        Json::Value item(Json::objectValue);
        item["pid"] = Json::Int64(source.pid);
        item["user"] = source.user;
        item["name"] = source.name;
        item["command"] = source.command;
        item["cpuPercent"] = source.cpuPercent;
        item["memoryPercent"] = source.memoryPercent;
        item["residentBytes"] = Json::UInt64(source.residentBytes);
        items.append(std::move(item));
    }
    root["processes"] = std::move(items);

    Json::Value monitor(Json::objectValue);
    monitor["updates"] = Json::UInt64(statistics.updates);
    monitor["cacheHits"] = Json::UInt64(statistics.cacheHits);
    monitor["failedUpdates"] = Json::UInt64(statistics.failedUpdates);
    monitor["lastUpdateDurationMs"] = Json::Int64(statistics.lastUpdateDuration.count());
    monitor["snapshotAgeMs"] = Json::Int64(statistics.snapshotAge.count());
    monitor["refreshIntervalMs"] = Json::Int64(statistics.refreshInterval.count());
    monitor["healthy"] = statistics.healthy;
    if (statistics.lastSuccessfulUpdate.time_since_epoch() !=
        std::chrono::system_clock::duration::zero()) {
        const auto last = std::chrono::duration_cast<std::chrono::seconds>(
            statistics.lastSuccessfulUpdate.time_since_epoch());
        monitor["lastSuccessfulUpdate"] = Json::Int64(last.count());
    } else {
        monitor["lastSuccessfulUpdate"] = Json::nullValue;
    }
    root["monitor"] = std::move(monitor);

    return root;
}

} // namespace

drogular::ActionResult ProcessStatusAction::handle(drogular::ActionContext& context) {
    const auto service = context.requireService<ProcessService>();
    const auto processes = service->snapshot();
    return drogular::ActionResult::json(toJson(processes, service->statistics()));
}

} // namespace system_monitor