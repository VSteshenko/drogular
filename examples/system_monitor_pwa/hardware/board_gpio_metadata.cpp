#include "board_gpio_metadata.hpp"

#include <array>
#include <string_view>

namespace system_monitor {

namespace {

struct HeaderPinMapping {
    std::uint32_t gpio;
    std::uint32_t physicalPin;
};

constexpr std::array<HeaderPinMapping, 28> kRaspberryPi40PinHeader{{
    {2, 3},
    {3, 5},
    {4, 7},
    {14, 8},
    {15, 10},
    {17, 11},
    {18, 12},
    {27, 13},
    {22, 15},
    {23, 16},
    {24, 18},
    {10, 19},
    {9, 21},
    {25, 22},
    {11, 23},
    {8, 24},
    {7, 26},
    {0, 27},
    {1, 28},
    {5, 29},
    {6, 31},
    {12, 32},
    {13, 33},
    {19, 35},
    {16, 36},
    {26, 37},
    {20, 38},
    {21, 40}
}};

bool isRaspberryPi4(std::string_view model) {
    return model.starts_with("Raspberry Pi 4 ");
}

std::optional<std::uint32_t> raspberryPiHeaderPin(std::uint32_t gpio) {
    for (const auto& mapping : kRaspberryPi40PinHeader) {
        if (mapping.gpio == gpio) {
            return mapping.physicalPin;
        }
    }
    return std::nullopt;
}

} // namespace

BoardGpioMetadata::BoardGpioMetadata(BoardKind kind) noexcept
    : kind_(kind) {
}

BoardGpioMetadata BoardGpioMetadata::fromSystemSnapshot(const SystemSnapshot& snapshot) {
    if (snapshot.raspberryPi && isRaspberryPi4(snapshot.raspberryPi->model)) {
        return BoardGpioMetadata(BoardKind::RaspberryPi4);
    }

    return {};
}

GpioBoardLineMetadata BoardGpioMetadata::line(std::string_view chip,
                                              std::uint32_t offset) const noexcept
{
    if (kind_ != BoardKind::RaspberryPi4) {
        return {};
    }

    if (chip == "gpiochip0") {
        if (const auto physicalPin = raspberryPiHeaderPin(offset)) {
            return {
                .exposure = GpioExposure::Header,
                .physicalHeaderPin = physicalPin
            };
        }

        return {
            .exposure = GpioExposure::Internal,
            .physicalHeaderPin = std::nullopt
        };
    }

    if (chip == "gpiochip1") {
        // Raspberry Pi 4 expansion GPIO describes board-level control signals,
        // not the user-facing 40-pin header.
        return {
            .exposure = GpioExposure::OnBoard,
            .physicalHeaderPin = std::nullopt
        };
    }

    return {};
}

bool BoardGpioMetadata::available() const noexcept {
    return kind_ != BoardKind::Unknown;
}

GpioExposure combineGpioExposure(GpioExposure current, GpioExposure next) noexcept {
    if (next == GpioExposure::Unknown) {
        return current;
    }
    if (current == GpioExposure::Unknown) {
        return next;
    }
    if (current == next) {
        return current;
    }
    return GpioExposure::Mixed;
}

std::string_view gpioExposureName(GpioExposure exposure) noexcept {
    switch (exposure) {
        case GpioExposure::Header:
            return "header";

        case GpioExposure::OnBoard:
            return "onboard";

        case GpioExposure::Internal:
            return "internal";

        case GpioExposure::Mixed:
            return "mixed";

        case GpioExposure::Unknown:
        default:
            return "unknown";
    }
}

} // namespace system_monitor