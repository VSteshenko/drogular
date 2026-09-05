#pragma once

#include "hardware/board_gpio_metadata.hpp"
#include "services/gpio_service.hpp"
#include "services/system_monitor.hpp"

#include <drogular/render_context.hpp>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <optional>
#include <string>

namespace system_monitor::hardware_fragment_support {

inline BoardGpioMetadata boardMetadata(drogular::RenderContext& context) {
    BoardGpioMetadata result;
    if (const auto monitor = context.service<SystemMonitor>()) {
        try {
            result = BoardGpioMetadata::fromSystemSnapshot(monitor->snapshot());
        } catch (...) {
        }
    }

    return result;
}

inline std::optional<GpioSnapshot> gpioSnapshot(drogular::RenderContext& context) {
    if (const auto service = context.service<GpioService>();
        service && service->available()
    ) {
        try {
            return service->snapshot();
        } catch (...) {
        }
    }

    return std::nullopt;
}

inline std::string age(drogular::RenderContext& context, std::chrono::milliseconds value) {
    auto seconds =
        std::max<std::int64_t>(0, value.count()) / 1000;
    if (seconds < 60) {
        return std::to_string(seconds) + context.translate("client.second_short");
    }

    auto minutes = seconds / 60;
    if (minutes < 60) {
        return std::to_string(minutes) + context.translate("client.minute_short");
    }

    return std::to_string(minutes / 60) + context.translate("client.hour_short");
}

inline std::string clock(std::chrono::system_clock::time_point value) {
    if (value.time_since_epoch() == std::chrono::system_clock::duration::zero()) {
        return {};
    }

    auto t = std::chrono::system_clock::to_time_t(value);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    char buffer[16]{};

    return std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &local)
        ? std::string(buffer)
        : std::string{};
}
inline std::string exposure(GpioExposure value, std::optional<std::uint32_t> pin = std::nullopt) {
    switch (value) {
        case GpioExposure::Header:
            return pin ? "40-pin · pin " + std::to_string(*pin) : "40-pin header";

        case GpioExposure::OnBoard:
            return "onboard";

        case GpioExposure::Internal:
            return "internal";

        case GpioExposure::Mixed:
            return "mixed";

        case GpioExposure::Unknown:
            return "unknown";
    }

    return "unknown";
}

} // namespace system_monitor::hardware_fragment_support