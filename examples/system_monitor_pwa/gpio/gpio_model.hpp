#pragma once

#include <cstdint>
#include <string>

namespace system_monitor {

struct GpioChipInfo {
    std::string name;
    std::string label;
    std::uint32_t lineCount{0};
};

} // namespace system_monitor