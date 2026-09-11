#include "gpio_fragment_component.hpp"
#include "hardware/board_gpio_metadata.hpp"
#include "services/gpio_service.hpp"
#include "services/system_monitor.hpp"

#include <drogular/render_context.hpp>

#include <json/json.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <string>
#include <string_view>
#include <utility>

namespace system_monitor {
namespace {

std::string requestParameter(const drogular::RenderContext& context,
                             const std::string& name,
                             std::string fallback = {}) {
    const auto request = context.request();
    if (request == nullptr) return fallback;
    const auto value = request->getParameter(name);
    return value.empty() ? std::move(fallback) : value;
}

std::string directionName(GpioLineDirection direction) {
    switch (direction) {
        case GpioLineDirection::Input: return "input";
        case GpioLineDirection::Output: return "output";
        case GpioLineDirection::Unknown: return "unknown";
    }
    return "unknown";
}

std::string directionVariant(GpioLineDirection direction) {
    switch (direction) {
        case GpioLineDirection::Input:
            return "dg-badge-info";

        case GpioLineDirection::Output:
            return "dg-badge-warning";

        case GpioLineDirection::Unknown:
            return "dg-badge-neutral";
    }

    return "dg-badge-neutral";
}

std::string flags(const GpioLineInfo& line) {
    std::string result;
    if (line.activeLow) result = "active-low";
    if (line.drive != GpioLineDrive::PushPull) {
        const std::string_view drive = line.drive == GpioLineDrive::OpenDrain
            ? "open-drain"
            : line.drive == GpioLineDrive::OpenSource ? "open-source" : "unknown";
        if (!result.empty()) result += ", ";
        result += drive;
    }
    return result.empty() ? "—" : result;
}

std::string exposureText(const GpioBoardLineMetadata& metadata) {
    switch (metadata.exposure) {
        case GpioExposure::Header:
            return metadata.physicalHeaderPin
                ? "40-pin · pin " + std::to_string(*metadata.physicalHeaderPin)
                : "40-pin header";
        case GpioExposure::OnBoard: return "onboard";
        case GpioExposure::Internal: return "internal";
        case GpioExposure::Mixed: return "mixed";
        case GpioExposure::Unknown: return "unknown";
    }
    return "unknown";
}

bool matchesFilter(const GpioLineInfo& line, std::string_view filter) {
    if (filter == "active") return line.used || line.alternateFunction;
    if (filter == "free") return !line.used && !line.alternateFunction;
    return true;
}

std::string formatAge(drogular::RenderContext& context, std::chrono::milliseconds age) {
    const auto seconds = std::max<std::int64_t>(0, age.count()) / 1000;
    if (seconds < 60) return std::to_string(seconds) + context.translate("client.second_short");
    const auto minutes = seconds / 60;
    if (minutes < 60) return std::to_string(minutes) + context.translate("client.minute_short");
    return std::to_string(minutes / 60) + context.translate("client.hour_short");
}

std::string formatClock(std::chrono::system_clock::time_point time) {
    if (time.time_since_epoch() == std::chrono::system_clock::duration::zero()) return {};
    const auto value = std::chrono::system_clock::to_time_t(time);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &value);
#else
    localtime_r(&value, &local);
#endif
    char buffer[16]{};
    return std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &local) == 0 ? std::string{} : std::string(buffer);
}

} // namespace

void GpioFragmentComponent::onInit(drogular::RenderContext& context) {
    const auto service = context.requireService<GpioService>();
    const auto snapshot = service->snapshot();
    const auto statistics = service->statistics();

    auto filter = requestParameter(context, "filter", "active");
    if (filter != "all" && filter != "active" && filter != "free") filter = "active";

    BoardGpioMetadata boardMetadata;
    if (const auto monitor = context.service<SystemMonitor>()) {
        try { boardMetadata = BoardGpioMetadata::fromSystemSnapshot(monitor->snapshot()); }
        catch (...) { }
    }

    std::size_t totalLines = 0;
    std::size_t usedLines = 0;
    Json::Value chips(Json::arrayValue);
    for (const auto& source : snapshot.chips) {
        Json::Value chip(Json::objectValue);
        chip["name"] = source.chip.name;
        chip["label"] = source.chip.label.empty() ? context.translate("client.unlabelled_chip") : source.chip.label;
        chip["preserveKey"] = "gpio-chip:" + source.chip.name;
        chip["lineCount"] = source.chip.lineCount;

        Json::Value lines(Json::arrayValue);
        std::size_t visibleCount = 0;
        std::size_t chipUsed = 0;
        for (const auto& sourceLine : source.lines) {
            ++totalLines;
            if (sourceLine.used) { ++usedLines; ++chipUsed; }
            if (!matchesFilter(sourceLine, filter)) continue;
            ++visibleCount;
            const auto metadata = boardMetadata.line(source.chip.name, sourceLine.offset);
            Json::Value line(Json::objectValue);
            line["offset"] = sourceLine.offset;
            line["name"] = sourceLine.name.empty() ? context.translate("client.unnamed") : sourceLine.name;
            line["consumer"] = sourceLine.consumer.empty() ? "—" : sourceLine.consumer;
            line["function"] = sourceLine.function.empty() ? "GPIO" : sourceLine.function;
            line["direction"] = directionName(sourceLine.direction);
            line["directionVariant"] = directionVariant(sourceLine.direction);
            line["flags"] = flags(sourceLine);
            line["used"] = sourceLine.used;
            line["alternateFunction"] = sourceLine.alternateFunction;
            line["exposure"] = exposureText(metadata);
            line["functionClass"] = sourceLine.alternateFunction ? "gpio-function-active" : "";
            line["consumerClass"] = sourceLine.consumer.empty() ? "muted" : "";
            line["flagsClass"] = flags(sourceLine) == "—" ? "muted" : "";
            line["stateVariant"] = sourceLine.used ? "dg-badge-warning" : "dg-badge-success";
            line["stateText"] = context.translate(sourceLine.used ? "client.used" : "client.free");
            lines.append(std::move(line));
        }
        chip["hasLines"] = visibleCount != 0;
        chip["emptyText"] = context.translate(
            filter == "active" ? "client.no_active_lines" :
            filter == "free" ? "client.no_free_lines" : "client.no_gpio_lines");
        chip["lines"] = std::move(lines);
        if (filter == "all") {
            chip["stats"] = std::to_string(source.chip.lineCount) + " " + context.translate("client.lines") +
                            " · " + std::to_string(chipUsed) + " " + context.translate("dashboard.used");
        } else {
            chip["stats"] = std::to_string(visibleCount) + " / " + std::to_string(source.chip.lineCount) +
                            " " + context.translate("client.shown");
        }
        chips.append(std::move(chip));
    }

    context.set("gpioAvailable", statistics.available);
    context.set("hasGpioChips", !snapshot.chips.empty());
    context.set("gpioFilter", filter);
    context.set("filterAll", filter == "all");
    context.set("filterActive", filter == "active");
    context.set("filterFree", filter == "free");
    context.set("filterAllClass", filter == "all" ? "is-active" : "");
    context.set("filterActiveClass", filter == "active" ? "is-active" : "");
    context.set("filterFreeClass", filter == "free" ? "is-active" : "");
    context.set("gpioChips", std::move(chips));
    context.set("gpioSummary", std::to_string(snapshot.chips.size()) + " " + context.translate("client.chips") +
                               " · " + std::to_string(totalLines) + " " + context.translate("client.lines") +
                               " · " + std::to_string(usedLines) + " " + context.translate("dashboard.used"));
    if (!statistics.healthy && statistics.available) {
        context.set("gpioStatus", context.translate("status.stale") + " · " + formatAge(context, statistics.snapshotAge) + " " + context.translate("client.old"));
    } else {
        const auto updated = formatClock(statistics.lastSuccessfulUpdate);
        context.set("gpioStatus", updated.empty() ? "GPIO " + context.translate("client.inventory_available")
                                                   : context.translate("client.updated") + " " + updated);
    }
}

} // namespace system_monitor