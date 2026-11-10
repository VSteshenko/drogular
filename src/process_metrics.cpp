#include <drogular/process_metrics.hpp>

#include <chrono>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/resource.h>
#include <sys/time.h>

#if defined(__linux__)
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <unistd.h>
#endif

namespace drogular {
namespace {

std::uint64_t timeMicros(const timeval& value) noexcept {
    return static_cast<std::uint64_t>(value.tv_sec) * 1000000ULL +
           static_cast<std::uint64_t>(value.tv_usec);
}

#if defined(__linux__)
std::uint64_t processUptimeMs() {
    // Field 22 of /proc/self/stat is the start time in clock ticks since boot.
    std::ifstream stat("/proc/self/stat");
    std::string line;
    if (!std::getline(stat, line)) {
        return 0;
    }
    const auto closing = line.rfind(')');
    if (closing == std::string::npos) {
        return 0;
    }
    std::istringstream values(line.substr(closing + 2));
    std::string token;
    std::uint64_t startTicks = 0;
    for (int field = 3; field <= 22; ++field) {
        if (!(values >> token)) {
            return 0;
        }
        if (field == 22) {
            try {
                startTicks = std::stoull(token);
            } catch (...) {
                return 0;
            }
        }
    }
    std::ifstream uptimeFile("/proc/uptime");
    double uptime = 0;
    if (!(uptimeFile >> uptime)) {
        return 0;
    }
    const long ticksPerSecond = sysconf(_SC_CLK_TCK);
    if (ticksPerSecond <= 0) {
        return 0;
    }
    const double duration = uptime - static_cast<double>(startTicks) / static_cast<double>(ticksPerSecond);
    return duration > 0 ? static_cast<std::uint64_t>(duration * 1000) : 0;
}

bool readProcessMemory(ProcessMetricsSnapshot& result) {
    std::ifstream status("/proc/self/status");
    if (!status) {
        return false;
    }
    std::string line;
    bool residentFound = false;
    while (std::getline(status, line)) {
        std::istringstream in(line);
        std::string key;
        std::uint64_t value = 0;
        if (!(in >> key >> value)) {
            continue;
        }
        if (key == "VmRSS:") {
            result.residentBytes = value * 1024;
            residentFound = true;
        } else if (key == "VmHWM:") {
            result.peakResidentBytes = value * 1024;
        } else if (key == "Threads:") {
            result.threadCount = value;
        }
    }
    return residentFound;
}
#elif defined(__APPLE__)
std::uint64_t processUptimeMs() {
    const int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid() };
    kinfo_proc info{};
    size_t size = sizeof(info);
    if (sysctl(const_cast<int*>(mib), 4, &info, &size, nullptr, 0) != 0 ||
        size != sizeof(info)
    ) {
        return 0;
    }
    timeval now{};
    gettimeofday(&now, nullptr);
    const auto started = info.kp_proc.p_starttime;
    const double elapsed = static_cast<double>(now.tv_sec - started.tv_sec) +
        static_cast<double>(now.tv_usec - started.tv_usec) / 1000000.0;
    return elapsed > 0 ? static_cast<std::uint64_t>(elapsed * 1000) : 0;
}

bool readProcessMemory(ProcessMetricsSnapshot& result) {
    mach_task_basic_info_data_t info{};
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
                  reinterpret_cast<task_info_t>(&info), &count) != KERN_SUCCESS
    ) {
        return false;
    }
    result.residentBytes = info.resident_size;
    thread_act_array_t threads = nullptr;
    mach_msg_type_number_t threadCount = 0;
    if (task_threads(mach_task_self(), &threads, &threadCount) == KERN_SUCCESS) {
        result.threadCount = threadCount;
        for (mach_msg_type_number_t i = 0; i < threadCount; ++i) {
            mach_port_deallocate(mach_task_self(), threads[i]);
        }
        vm_deallocate(mach_task_self(), reinterpret_cast<vm_address_t>(threads),
                      threadCount * sizeof(thread_act_t));
    }
    return true;
}
#else
std::uint64_t processUptimeMs() {
    return 0;
}
bool readProcessMemory(ProcessMetricsSnapshot&) {
    return false;
}
#endif

} // namespace

ProcessMetricsSnapshot ProcessMetricsProvider::snapshot() const noexcept {
    std::lock_guard lock(mutex_);
    ProcessMetricsSnapshot result;
    try {
        result.available = readProcessMemory(result);
        result.uptimeMs = processUptimeMs();
        rusage usage{};
        if (getrusage(RUSAGE_SELF, &usage) == 0) {
            const auto cpuUs = timeMicros(usage.ru_utime) + timeMicros(usage.ru_stime);
            result.cpuTimeMs = cpuUs / 1000;
#if defined(__APPLE__)
            // macOS ru_maxrss is measured in bytes, unlike Linux (KiB).
            result.peakResidentBytes = static_cast<std::uint64_t>(usage.ru_maxrss);
#elif defined(__linux__)
            if (!result.peakResidentBytes) {
                result.peakResidentBytes = static_cast<std::uint64_t>(usage.ru_maxrss) * 1024;
            }
#endif
            const auto now = std::chrono::steady_clock::now();
            if (lastSample_ && cpuUs >= lastCpuUs_) {
                const auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(
                    now - *lastSample_).count();
                if (elapsedUs > 0) {
                    result.cpuPercent = 100.0 *
                        static_cast<double>(cpuUs - lastCpuUs_) /
                        static_cast<double>(elapsedUs);
                }
            }
            lastCpuUs_ = cpuUs;
            lastSample_ = now;
        }
    } catch (...) {
        // Diagnostics must never disrupt application request processing.
        result.available = false;
    }
    return result;
}

} // namespace drogular