#pragma once

#include <cstdint>
#include <string>

namespace system_monitor {

enum class GpioLineDirection {
    Unknown,
    Input,
    Output
};

enum class GpioLineDrive {
    Unknown,
    PushPull,
    OpenDrain,
    OpenSource
};

struct GpioChipInfo {
    std::string name;
    std::string label;
    std::uint32_t lineCount{0};
};

struct GpioLineInfo {
    std::uint32_t offset{0};
    std::string name;
    std::string consumer;
    std::string function;
    GpioLineDirection direction{GpioLineDirection::Unknown};
    GpioLineDrive drive{GpioLineDrive::PushPull};
    bool activeLow{false};
    bool used{false};
    bool alternateFunction{false};
};

} // namespace system_monitor