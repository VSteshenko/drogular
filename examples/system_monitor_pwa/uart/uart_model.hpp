#pragma once

#include <string>
#include <vector>

namespace system_monitor {

struct UartDeviceInfo {
    std::string name;
    std::string path;
    std::vector<std::string> aliases;
};

struct UartSnapshot {
    std::vector<UartDeviceInfo> devices;
};

} // namespace system_monitor