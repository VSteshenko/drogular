#include "gpio_status_action.hpp"
#include "services/gpio_service.hpp"

#include <json/json.h>

#include <chrono>
#include <string_view>

namespace system_monitor {

namespace {

std::string_view directionName(GpioLineDirection direction) {
    switch (direction) {
        case GpioLineDirection::Input:
            return "input";

        case GpioLineDirection::Output:
            return "output";

        case GpioLineDirection::Unknown:
            return "unknown";
    }
    return "unknown";
}

std::string_view driveName(GpioLineDrive drive) {
    switch (drive) {
        case GpioLineDrive::PushPull:
            return "push-pull";

        case GpioLineDrive::OpenDrain:
            return "open-drain";

        case GpioLineDrive::OpenSource:
            return "open-source";

        case GpioLineDrive::Unknown:
            return "unknown";
    }
    return "unknown";
}

Json::Value toJson(
    const GpioSnapshot& snapshot,
    const GpioServiceStatistics& statistics
) {
    Json::Value root(Json::objectValue);

    const auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch());
    root["timestamp"] = Json::Int64(now.count());
    root["available"] = statistics.available;

    Json::Value chips(Json::arrayValue);
    for (const auto& source : snapshot.chips) {
        Json::Value chip(Json::objectValue);
        chip["name"] = source.chip.name;
        chip["label"] = source.chip.label;
        chip["lineCount"] = source.chip.lineCount;

        Json::Value lines(Json::arrayValue);
        for (const auto& sourceLine : source.lines) {
            Json::Value line(Json::objectValue);
            line["offset"] = sourceLine.offset;
            line["name"] = sourceLine.name;
            line["consumer"] = sourceLine.consumer;
            line["direction"] = std::string(directionName(sourceLine.direction));
            line["drive"] = std::string(driveName(sourceLine.drive));
            line["activeLow"] = sourceLine.activeLow;
            line["used"] = sourceLine.used;
            lines.append(std::move(line));
        }

        chip["lines"] = std::move(lines);
        chips.append(std::move(chip));
    }
    root["chips"] = std::move(chips);

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

drogular::ActionResult GpioStatusAction::handle(
    drogular::ActionContext& context
) {
    const auto service = context.requireService<GpioService>();
    const auto snapshot = service->snapshot();
    return drogular::ActionResult::json(toJson(snapshot, service->statistics()));
}

} // namespace system_monitor