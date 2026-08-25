#pragma once

#include <cstdint>
#include <string>
#include <utility>

namespace system_monitor {

enum class MonitorTargetType {
    Local,
    Ssh
};

struct SshTargetOptions {
    std::string host;
    std::uint16_t port{22};
    std::string user;
    std::string identityFile;
    std::string knownHostsFile;
};

struct MonitorTarget {
    MonitorTargetType type{MonitorTargetType::Local};
    SshTargetOptions ssh;

    [[nodiscard]] static MonitorTarget local() {
        return {};
    }

    [[nodiscard]] static MonitorTarget remote(SshTargetOptions options) {
        MonitorTarget target;
        target.type = MonitorTargetType::Ssh;
        target.ssh = std::move(options);
        return target;
    }
};

} // namespace system_monitor