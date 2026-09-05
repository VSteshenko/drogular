#include "system_fragment_component.hpp"

#include "services/system_monitor.hpp"
#include "ui/system_formatters.hpp"

#include <drogular/render_context.hpp>

#include <json/json.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <cstdint>
#include <string>

namespace system_monitor {
namespace {

std::string formatAge(
    drogular::RenderContext& context,
    std::chrono::milliseconds age) {
    const auto seconds = std::max<std::int64_t>(0, age.count()) / 1000;
    if (seconds < 60) {
        return std::to_string(seconds) + context.translate("client.second_short");
    }
    const auto minutes = seconds / 60;
    if (minutes < 60) {
        return std::to_string(minutes) + context.translate("client.minute_short");
    }

    return std::to_string(minutes / 60) + context.translate("client.hour_short");
}

std::string formatClock(std::chrono::system_clock::time_point time) {
    if (time.time_since_epoch() == std::chrono::system_clock::duration::zero()) {
        return {};
    }

    const auto value = std::chrono::system_clock::to_time_t(time);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &value);
#else
    localtime_r(&value, &local);
#endif

    char buffer[16]{};
    if (std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &local) == 0) {
        return {};
    }
    return buffer;
}

} // namespace

void SystemFragmentComponent::onInit(drogular::RenderContext& context) {
    const auto monitor = context.requireService<SystemMonitor>();
    const auto snapshot = monitor->snapshot();
    const auto statistics = monitor->statistics();
    const auto memoryPercent = ui::percentOf(
        snapshot.memory.usedBytes,
        snapshot.memory.totalBytes);

    context.set("hostname", snapshot.system.hostname);
    context.set("operatingSystem", snapshot.system.operatingSystem);
    context.set("kernel", snapshot.system.kernel);
    context.set("architecture", snapshot.system.architecture);
    context.set("uptime", ui::formatDuration(snapshot.system.uptimeSeconds));

    context.set("cpuUsage", ui::formatPercent(snapshot.cpu.usagePercent));
    context.set("cpuUsageValue", snapshot.cpu.usagePercent);
    context.set("cpuCores", static_cast<int>(snapshot.cpu.logicalCores));
    context.set("load1", snapshot.cpu.load1);
    context.set("load5", snapshot.cpu.load5);
    context.set("load15", snapshot.cpu.load15);

    context.set("memoryUsed", ui::formatBytes(snapshot.memory.usedBytes));
    context.set("memoryTotal", ui::formatBytes(snapshot.memory.totalBytes));
    context.set("memoryAvailable", ui::formatBytes(snapshot.memory.availableBytes));
    context.set("memoryUsage", ui::formatPercent(memoryPercent));
    context.set("memoryUsageValue", memoryPercent);

    Json::Value disks(Json::arrayValue);
    for (const auto& source : snapshot.disks) {
        const auto diskPercent = ui::percentOf(source.usedBytes, source.totalBytes);
        Json::Value disk;
        disk["device"] = source.device;
        disk["mountPoint"] = source.mountPoint;
        disk["fileSystem"] = source.fileSystem;
        disk["used"] = ui::formatBytes(source.usedBytes);
        disk["total"] = ui::formatBytes(source.totalBytes);
        disk["available"] = ui::formatBytes(source.availableBytes);
        disk["usage"] = ui::formatPercent(diskPercent);
        disk["usageValue"] = diskPercent;
        disks.append(std::move(disk));
    }
    context.set("hasDisks", !snapshot.disks.empty());
    context.set("disks", std::move(disks));

    context.set("hasRaspberryPi", snapshot.raspberryPi.has_value());
    if (snapshot.raspberryPi) {
        context.set("raspberryPiModel", snapshot.raspberryPi->model);
        context.set("raspberryPiRevision", snapshot.raspberryPi->revision);
        context.set("raspberryPiSerial", snapshot.raspberryPi->serial);
        context.set(
            "raspberryPiTemperature",
            snapshot.raspberryPi->temperatureCelsius
                ? ui::formatTemperature(*snapshot.raspberryPi->temperatureCelsius)
                : context.translate("status.unavailable"));
        context.set(
            "raspberryPiFrequency",
            snapshot.raspberryPi->cpuFrequencyHz
                ? ui::formatFrequency(*snapshot.raspberryPi->cpuFrequencyHz)
                : context.translate("status.unavailable"));

        context.set("hasRaspberryPiHealth", snapshot.raspberryPi->health.has_value());
        if (snapshot.raspberryPi->health) {
            const auto& health = *snapshot.raspberryPi->health;
            context.set(
                "raspberryPiHealthCurrent",
                health.underVoltage || health.frequencyCapped ||
                        health.throttled || health.softTemperatureLimit
                    ? context.translate("status.warning")
                    : context.translate("status.normal"));
            context.set(
                "raspberryPiHealthHistory",
                health.underVoltageOccurred ||
                        health.frequencyCappingOccurred ||
                        health.throttlingOccurred ||
                        health.softTemperatureLimitOccurred
                    ? context.translate("status.events_recorded")
                    : context.translate("status.no_events_recorded"));
        }
    }

    context.set("monitorHealthy", statistics.healthy);
    context.set("monitorConnectionState", std::string(statistics.healthy ? "live" : "stale"));
    if (statistics.healthy) {
        context.set("monitorStatusLabel", context.translate("status.live"));
        context.set("monitorStatusDetail", std::string{});
    } else {
        context.set(
            "monitorStatusLabel",
            context.translate("status.stale") + " · " +
                formatAge(context, statistics.snapshotAge) + " " +
                context.translate("client.old"));
        context.set("monitorStatusDetail", context.translate("client.target_stale"));
    }

    const auto updated = formatClock(statistics.lastSuccessfulUpdate);
    context.set(
        "lastUpdate",
        updated.empty() ? context.translate("dashboard.wait_first_refresh") : updated);
}

} // namespace system_monitor