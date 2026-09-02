#include "linux_system_metrics_provider.hpp"
#include "system/process_list_parser.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace system_monitor {
namespace {

std::uint64_t parseUInt64(std::string_view value, std::string_view field) {
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
        value.remove_prefix(1);
    }
    while (!value.empty() && (value.back() == ' ' || value.back() == '\t' || value.back() == '\r')) {
        value.remove_suffix(1);
    }

    std::uint64_t result = 0;
    const auto parsed = std::from_chars(value.data(), value.data() + value.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size()) {
        throw std::runtime_error("invalid numeric value for " + std::string(field));
    }
    return result;
}

std::uint64_t kibToBytes(std::uint64_t value) {
    constexpr std::uint64_t scale = 1024;
    if (value > std::numeric_limits<std::uint64_t>::max() / scale) {
        return std::numeric_limits<std::uint64_t>::max();
    }
    return value * scale;
}

std::unordered_map<std::string, std::uint64_t> parseMemInfo(std::string_view input) {
    std::unordered_map<std::string, std::uint64_t> values;
    std::istringstream stream{std::string(input)};
    std::string line;
    while (std::getline(stream, line)) {
        const auto colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        auto key = line.substr(0, colon);
        std::istringstream valueStream(line.substr(colon + 1));
        std::uint64_t value = 0;
        std::string unit;
        if (!(valueStream >> value)) {
            continue;
        }
        valueStream >> unit;
        if (unit == "kB") {
            value = kibToBytes(value);
        }
        values.emplace(std::move(key), value);
    }

    return values;
}

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

CommandResult requireCommand(SystemReader& reader, std::string_view command) {
    auto result = reader.execute(command);
    if (!result.succeeded()) {
        throw std::runtime_error("command failed: " + std::string(command));
    }

    return result;
}

} // namespace

LinuxSystemMetricsProvider::LinuxSystemMetricsProvider(std::shared_ptr<SystemReader> reader)
    : reader_(std::move(reader)),
      raspberryPiProbe_(reader_)
{
    if (!reader_) {
        throw std::invalid_argument("LinuxSystemMetricsProvider requires a SystemReader");
    }
}

SystemSnapshot LinuxSystemMetricsProvider::snapshot() {
    SystemSnapshot result;
    result.cpu = readCpu();
    result.memory = readMemory();
    result.disks = readDisks();
    result.system = readSystem();
    result.raspberryPi = raspberryPiProbe_.read();
    if (result.raspberryPi && result.raspberryPi->temperatureCelsius) {
        result.cpu.temperatureCelsius = result.raspberryPi->temperatureCelsius;
    }
    return result;
}

std::vector<ProcessInfo> LinuxSystemMetricsProvider::processes() {
    const auto command = requireCommand(
        *reader_,
        "LC_ALL=C ps -eo pid=,user=,%cpu=,%mem=,rss=,comm=,args=");
    return parseProcessList(command.standardOutput);
}

CpuInfo LinuxSystemMetricsProvider::readCpu() {
    CpuInfo info;

    {
        std::istringstream cpuInfo(reader_->readFile("/proc/cpuinfo"));
        std::string line;
        while (std::getline(cpuInfo, line)) {
            if (line.rfind("processor", 0) == 0 && line.find(':') != std::string::npos) {
                ++info.logicalCores;
            }
        }
    }

    {
        std::istringstream load(reader_->readFile("/proc/loadavg"));
        load >> info.load1 >> info.load5 >> info.load15;
        if (!load) {
            throw std::runtime_error("invalid /proc/loadavg");
        }
    }

    const auto current = readCpuTicks();
    std::lock_guard lock(cpuMutex_);
    if (previousCpuTicks_ &&
        current.active >= previousCpuTicks_->active &&
        current.idle >= previousCpuTicks_->idle) {
        const auto activeDelta = current.active - previousCpuTicks_->active;
        const auto idleDelta = current.idle - previousCpuTicks_->idle;
        const auto totalDelta = activeDelta + idleDelta;
        if (totalDelta != 0) {
            info.usagePercent = 100.0 * static_cast<double>(activeDelta) /
                static_cast<double>(totalDelta);
        }
    } else {
        const auto total = current.active + current.idle;
        if (total != 0) {
            info.usagePercent = 100.0 * static_cast<double>(current.active) /
                static_cast<double>(total);
        }
    }
    previousCpuTicks_ = current;
    info.usagePercent = std::clamp(info.usagePercent, 0.0, 100.0);

    return info;
}



MemoryInfo LinuxSystemMetricsProvider::readMemory() const {
    const auto values =
        parseMemInfo(reader_->readFile("/proc/meminfo"));
    const auto totalIt = values.find("MemTotal");
    if (totalIt == values.end()) {
        throw std::runtime_error("MemTotal missing from /proc/meminfo");
    }

    MemoryInfo info;
    info.totalBytes = totalIt->second;

    const auto availableIt = values.find("MemAvailable");
    if (availableIt != values.end()) {
        info.availableBytes = availableIt->second;
    } else {
        const auto get = [&](std::string_view key) {
            const auto it = values.find(std::string(key));
            return it == values.end() ? std::uint64_t{0} : it->second;
        };
        info.availableBytes =
            get("MemFree") + get("Buffers") + get("Cached") + get("SReclaimable");
    }

    info.availableBytes = std::min(info.availableBytes, info.totalBytes);
    info.usedBytes = info.totalBytes - info.availableBytes;

    return info;
}

std::vector<DiskInfo> LinuxSystemMetricsProvider::readDisks() const {
    const auto command = requireCommand(*reader_, "df -P -B1 /");
    std::istringstream stream(command.standardOutput);
    std::string header;
    std::getline(stream, header);

    DiskInfo disk;
    std::uint64_t used = 0;
    std::uint64_t available = 0;
    std::uint64_t total = 0;
    std::string capacity;
    if (!(stream >> disk.device >> total >> used >> available >> capacity >> disk.mountPoint)) {
        throw std::runtime_error("invalid df output");
    }
    disk.fileSystem.clear();
    disk.totalBytes = total;
    disk.usedBytes = std::min(used, total);
    disk.availableBytes = std::min(available, total);

    return {std::move(disk)};
}

SystemInfo LinuxSystemMetricsProvider::readSystem() const {
    SystemInfo info;
    info.hostname = trim(requireCommand(*reader_, "hostname").standardOutput);
    info.operatingSystem = "Linux";
    const auto kernelName = trim(requireCommand(*reader_, "uname -s").standardOutput);
    const auto kernelRelease = trim(requireCommand(*reader_, "uname -r").standardOutput);
    info.kernel = kernelName + " " + kernelRelease;
    info.architecture = trim(requireCommand(*reader_, "uname -m").standardOutput);

    std::istringstream uptime(reader_->readFile("/proc/uptime"));
    double uptimeSeconds = 0.0;
    if (!(uptime >> uptimeSeconds) || !std::isfinite(uptimeSeconds) || uptimeSeconds < 0.0) {
        throw std::runtime_error("invalid /proc/uptime");
    }
    info.uptimeSeconds = static_cast<std::uint64_t>(uptimeSeconds);

    return info;
}

LinuxSystemMetricsProvider::CpuTicks LinuxSystemMetricsProvider::readCpuTicks() const {
    std::istringstream stream(reader_->readFile("/proc/stat"));
    std::string cpu;
    std::uint64_t user = 0;
    std::uint64_t nice = 0;
    std::uint64_t system = 0;
    std::uint64_t idle = 0;
    std::uint64_t iowait = 0;
    std::uint64_t irq = 0;
    std::uint64_t softirq = 0;
    std::uint64_t steal = 0;
    if (!(stream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal) || cpu != "cpu") {
        throw std::runtime_error("invalid /proc/stat");
    }

    CpuTicks ticks;
    ticks.active = user + nice + system + irq + softirq + steal;
    ticks.idle = idle + iowait;

    return ticks;
}

} // namespace system_monitor