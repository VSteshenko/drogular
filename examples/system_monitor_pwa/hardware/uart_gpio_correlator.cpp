#include "uart_gpio_correlator.hpp"

#include <charconv>
#include <cctype>
#include <map>
#include <string_view>

namespace system_monitor {

namespace {

bool parseFunction(std::string_view function, std::string& role, std::uint32_t& controller) {
    constexpr std::string_view roles[] = {"TXD", "RXD", "CTS", "RTS"};

    for (const auto candidate : roles) {
        if (!function.starts_with(candidate)) {
            continue;
        }

        const auto suffix = function.substr(candidate.size());
        if (suffix.empty()) {
            return false;
        }

        std::uint32_t value = 0;
        const auto [end, error] =
            std::from_chars(suffix.data(), suffix.data() + suffix.size(), value);
        if (error != std::errc{} || end != suffix.data() + suffix.size()) {
            return false;
        }

        role.assign(candidate);
        for (auto& ch : role) {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
        controller = value;

        return true;
    }

    return false;
}

} // namespace

std::vector<UartGpioGroup> UartGpioCorrelator::groups(const GpioSnapshot& gpioSnapshot) {
    std::map<std::uint32_t, UartGpioGroup> groups;
    for (const auto& chip : gpioSnapshot.chips) {
        for (const auto& line : chip.lines) {
            std::string role;
            std::uint32_t controller = 0;
            if (!parseFunction(line.function, role, controller)) {
                continue;
            }

            auto& group = groups[controller];
            group.controller = controller;
            group.pins.push_back(UartGpioPin{
                .role = std::move(role),
                .chip = chip.chip.name,
                .offset = line.offset,
                .name = line.name,
                .function = line.function,
                .consumer = line.consumer
            });
        }
    }

    std::vector<UartGpioGroup> result;
    result.reserve(groups.size());
    for (auto& [controller, group] : groups) {
        result.push_back(std::move(group));
    }
    return result;
}

} // namespace system_monitor