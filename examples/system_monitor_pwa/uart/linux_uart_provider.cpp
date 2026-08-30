#include "linux_uart_provider.hpp"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace system_monitor {

namespace {

constexpr std::string_view kInventoryCommand =
    "for p in /dev/ttyAMA* /dev/ttyS*; do "
    "[ -c \"$p\" ] && printf 'device\\t%s\\n' \"$p\"; "
    "done; "
    "for p in /dev/serial*; do "
    "[ -L \"$p\" ] && printf 'alias\\t%s\\t%s\\n' \"$p\" \"$(readlink -f \"$p\")\"; "
    "done";

std::string baseName(std::string_view path) {
    const auto slash = path.find_last_of('/');
    return std::string(slash == std::string_view::npos ? path : path.substr(slash + 1));
}

} // namespace

LinuxUartProvider::LinuxUartProvider(std::shared_ptr<SystemReader> reader)
    : reader_(std::move(reader))
{
    if (!reader_) {
        throw std::invalid_argument("LinuxUartProvider requires a SystemReader");
    }
}

std::vector<UartDeviceInfo> LinuxUartProvider::devices() {
    const auto result = reader_->execute(kInventoryCommand);
    if (!result.succeeded()) {
        throw std::runtime_error("UART device inventory failed with exit code " +
                                 std::to_string(result.exitCode) + ": " +
                                 (!result.standardError.empty() ? result.standardError : result.standardOutput));
    }

    std::map<std::string, UartDeviceInfo> byPath;
    std::string_view output(result.standardOutput);
    while (!output.empty()) {
        const auto newline = output.find('\n');
        auto line = output.substr(0, newline);
        if (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }

        const auto firstTab = line.find('\t');
        if (firstTab != std::string_view::npos) {
            const auto type = line.substr(0, firstTab);
            const auto rest = line.substr(firstTab + 1);
            if (type == "device" && rest.starts_with("/dev/")) {
                byPath.try_emplace(std::string(rest), UartDeviceInfo{
                    .name = baseName(rest),
                    .path = std::string(rest),
                    .aliases = {}
                });
            } else if (type == "alias") {
                const auto secondTab = rest.find('\t');
                if (secondTab != std::string_view::npos) {
                    const auto alias = rest.substr(0, secondTab);
                    const auto target = rest.substr(secondTab + 1);
                    if (alias.starts_with("/dev/serial") && target.starts_with("/dev/")) {
                        auto [it, inserted] =
                            byPath.try_emplace(std::string(target), UartDeviceInfo{
                                .name = baseName(target),
                                .path = std::string(target),
                                .aliases = {}
                            });
                        auto& aliases = it->second.aliases;
                        if (std::find(aliases.begin(), aliases.end(), alias) == aliases.end()) {
                            aliases.emplace_back(alias);
                        }
                    }
                }
            }
        }

        if (newline == std::string_view::npos) {
            break;
        }
        output.remove_prefix(newline + 1);
    }

    std::vector<UartDeviceInfo> devices;
    devices.reserve(byPath.size());
    for (auto& [path, device] : byPath) {
        std::sort(device.aliases.begin(), device.aliases.end());
        devices.push_back(std::move(device));
    }
    return devices;
}

} // namespace system_monitor