#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace system_monitor {

struct SpiDeviceInfo {
    std::uint32_t bus{0};
    std::uint32_t chipSelect{0};
    std::string path;
};

struct SpiSnapshot {
    std::vector<SpiDeviceInfo> devices;
};

} // namespace system_monitor