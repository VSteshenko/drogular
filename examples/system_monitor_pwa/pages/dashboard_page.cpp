#include "dashboard_page.hpp"

#include "services/system_monitor.hpp"
#include "ui/system_formatters.hpp"

#include <drogular/render_context.hpp>

#include <json/json.h>

#include <stdexcept>
#include <string>

namespace system_monitor {

void DashboardPage::onInit(drogular::RenderContext& context) {
    const auto monitor = context.service<SystemMonitor>();
    if (monitor == nullptr) {
        throw std::runtime_error("SystemMonitor service is not registered");
    }

    const auto snapshot = monitor->snapshot();
    const auto memoryPercent = ui::percentOf(
        snapshot.memory.usedBytes,
        snapshot.memory.totalBytes);

    context.set("title", std::string("Drogular System Monitor"));
    context.set("hostname", snapshot.system.hostname);
    context.set("operatingSystem", snapshot.system.operatingSystem);
    context.set("kernel", snapshot.system.kernel);
    context.set("architecture", snapshot.system.architecture);
    context.set("uptime", ui::formatDuration(snapshot.system.uptimeSeconds));

    context.set("cpuUsage", ui::formatPercent(snapshot.cpu.usagePercent));
    context.set("cpuUsageValue", snapshot.cpu.usagePercent);
    context.set(
        "cpuCores",
        static_cast<int>(snapshot.cpu.logicalCores));
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

    context.set("hasRaspberryPi", snapshot.raspberryPi.has_value());
    if (snapshot.raspberryPi) {
        context.set("raspberryPiModel", snapshot.raspberryPi->model);
        context.set("raspberryPiRevision", snapshot.raspberryPi->revision);
        context.set("raspberryPiSerial", snapshot.raspberryPi->serial);
        context.set(
            "raspberryPiTemperature",
            snapshot.raspberryPi->temperatureCelsius
                ? ui::formatTemperature(*snapshot.raspberryPi->temperatureCelsius)
                : std::string("Unavailable"));
        context.set(
            "raspberryPiFrequency",
            snapshot.raspberryPi->cpuFrequencyHz
                ? ui::formatFrequency(*snapshot.raspberryPi->cpuFrequencyHz)
                : std::string("Unavailable"));

        context.set("hasRaspberryPiHealth", snapshot.raspberryPi->health.has_value());
        if (snapshot.raspberryPi->health) {
            const auto& health = *snapshot.raspberryPi->health;
            context.set(
                "raspberryPiHealthCurrent",
                health.underVoltage || health.frequencyCapped ||
                        health.throttled || health.softTemperatureLimit
                    ? std::string("Warning")
                    : std::string("Normal"));
            context.set(
                "raspberryPiHealthHistory",
                health.underVoltageOccurred ||
                        health.frequencyCappingOccurred ||
                        health.throttlingOccurred ||
                        health.softTemperatureLimitOccurred
                    ? std::string("Events recorded")
                    : std::string("No events recorded"));
        }
    }

    context.set("hasDisks", !snapshot.disks.empty());
    context.set("disks", std::move(disks));
}

} // namespace system_monitor