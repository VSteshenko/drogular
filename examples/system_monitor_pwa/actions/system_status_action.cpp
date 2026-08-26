#include "system_status_action.hpp"

#include "services/system_monitor.hpp"
#include "ui/system_formatters.hpp"

#include <json/json.h>

#include <chrono>
#include <cstdint>

namespace system_monitor {
namespace {

Json::Value toJson(const SystemSnapshot& snapshot) {
    Json::Value root(Json::objectValue);

    const auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch());
    root["timestamp"] = Json::Int64(now.count());

    Json::Value cpu(Json::objectValue);
    cpu["usagePercent"] = snapshot.cpu.usagePercent;
    cpu["logicalCores"] = snapshot.cpu.logicalCores;
    cpu["load1"] = snapshot.cpu.load1;
    cpu["load5"] = snapshot.cpu.load5;
    cpu["load15"] = snapshot.cpu.load15;
    root["cpu"] = std::move(cpu);

    const auto memoryPercent = ui::percentOf(
        snapshot.memory.usedBytes,
        snapshot.memory.totalBytes);
    Json::Value memory(Json::objectValue);
    memory["totalBytes"] = Json::UInt64(snapshot.memory.totalBytes);
    memory["usedBytes"] = Json::UInt64(snapshot.memory.usedBytes);
    memory["availableBytes"] = Json::UInt64(snapshot.memory.availableBytes);
    memory["usagePercent"] = memoryPercent;
    root["memory"] = std::move(memory);

    Json::Value disks(Json::arrayValue);
    for (const auto& source : snapshot.disks) {
        Json::Value disk(Json::objectValue);
        disk["device"] = source.device;
        disk["mountPoint"] = source.mountPoint;
        disk["fileSystem"] = source.fileSystem;
        disk["totalBytes"] = Json::UInt64(source.totalBytes);
        disk["usedBytes"] = Json::UInt64(source.usedBytes);
        disk["availableBytes"] = Json::UInt64(source.availableBytes);
        disk["usagePercent"] = ui::percentOf(source.usedBytes, source.totalBytes);
        disks.append(std::move(disk));
    }
    root["disks"] = std::move(disks);

    Json::Value system(Json::objectValue);
    system["hostname"] = snapshot.system.hostname;
    system["operatingSystem"] = snapshot.system.operatingSystem;
    system["kernel"] = snapshot.system.kernel;
    system["architecture"] = snapshot.system.architecture;
    system["uptimeSeconds"] = Json::UInt64(snapshot.system.uptimeSeconds);
    root["system"] = std::move(system);

    return root;
}

} // namespace

drogular::ActionResult SystemStatusAction::handle(
    drogular::ActionContext& context
) {
    const auto monitor = context.requireService<SystemMonitor>();
    return drogular::ActionResult::json(toJson(monitor->snapshot()));
}

} // namespace system_monitor