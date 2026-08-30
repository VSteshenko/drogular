#pragma once

#include "system/system_snapshot.hpp"

#include <cstdint>
#include <optional>
#include <string_view>

namespace system_monitor {

enum class GpioExposure {
    Unknown,
    Header,
    OnBoard,
    Internal,
    Mixed
};

struct GpioBoardLineMetadata {
    GpioExposure exposure{GpioExposure::Unknown};
    std::optional<std::uint32_t> physicalHeaderPin;
};

class BoardGpioMetadata {
public:
    BoardGpioMetadata() = default;

    [[nodiscard]] static BoardGpioMetadata fromSystemSnapshot(const SystemSnapshot& snapshot);

    [[nodiscard]] GpioBoardLineMetadata line(std::string_view chip,
                                             std::uint32_t offset) const noexcept;

    [[nodiscard]] bool available() const noexcept;

private:
    enum class BoardKind {
        Unknown,
        RaspberryPi4
    };

    explicit BoardGpioMetadata(BoardKind kind) noexcept;

    BoardKind kind_{BoardKind::Unknown};
};

[[nodiscard]] GpioExposure combineGpioExposure(GpioExposure current,
                                               GpioExposure next) noexcept;
[[nodiscard]] std::string_view gpioExposureName(GpioExposure exposure) noexcept;

} // namespace system_monitor