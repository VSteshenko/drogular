#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace system_monitor {

struct I2cBusInfo {
    std::uint32_t number{0};
    std::string name;
    std::string type;
    std::string description;
    std::string algorithm;
};

struct I2cDeviceInfo {
    std::uint8_t address{0};
    bool claimedByKernel{false};
};

struct I2cBusSnapshot {
    I2cBusInfo bus;
    std::vector<I2cDeviceInfo> devices;
    bool scanned{false};
};

struct I2cSnapshot {
    std::vector<I2cBusSnapshot> buses;
};

} // namespace system_monitor