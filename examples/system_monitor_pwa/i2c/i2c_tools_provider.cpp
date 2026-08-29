#include "i2c_tools_provider.hpp"

#include <charconv>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace system_monitor {

namespace {

constexpr std::string_view kToolPath =
    "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin ";

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::vector<std::string> splitTabs(std::string_view value) {
    std::vector<std::string> fields;
    std::size_t position = 0;
    while (position <= value.size()) {
        const auto end = value.find('\t', position);
        fields.push_back(trim(std::string(value.substr(
            position,
            end == std::string_view::npos ? value.size() - position : end - position))));
        if (end == std::string_view::npos) {
            break;
        }
        position = end + 1;
    }
    return fields;
}

std::optional<I2cBusInfo> parseBus(std::string line) {
    line = trim(std::move(line));
    if (!line.starts_with("i2c-")) {
        return std::nullopt;
    }

    const auto fields = splitTabs(line);
    if (fields.size() < 3) {
        return std::nullopt;
    }

    const auto dash = fields[0].find('-');
    if (dash == std::string::npos || dash + 1 >= fields[0].size()) {
        return std::nullopt;
    }

    std::uint32_t number = 0;
    const auto numberText = std::string_view(fields[0]).substr(dash + 1);
    const auto parsed = std::from_chars(
        numberText.data(),
        numberText.data() + numberText.size(),
        number);
    if (parsed.ec != std::errc{} ||
        parsed.ptr != numberText.data() + numberText.size()) {
        return std::nullopt;
    }

    I2cBusInfo info;
    info.number = number;
    info.name = fields[0];
    info.type = fields.size() > 1 ? fields[1] : std::string{};
    info.description = fields.size() > 2 ? fields[2] : std::string{};
    info.algorithm = fields.size() > 3 ? fields[3] : std::string{};
    return info;
}

std::optional<std::uint32_t> parseHex(std::string_view text) {
    std::uint32_t value = 0;
    const auto parsed = std::from_chars(
        text.data(),
        text.data() + text.size(),
        value,
        16);
    if (parsed.ec != std::errc{} ||
        parsed.ptr != text.data() + text.size()) {
        return std::nullopt;
    }
    return value;
}

std::vector<I2cDeviceInfo> parseScan(std::string_view text) {
    std::vector<I2cDeviceInfo> devices;
    std::istringstream input{std::string(text)};
    std::string line;

    while (std::getline(input, line)) {
        if (line.size() < 3 || line[2] != ':') {
            continue;
        }

        const auto row = parseHex(std::string_view(line).substr(0, 2));
        if (!row || *row > 0x70) {
            continue;
        }

        const std::string_view cells(line.data() + 3, line.size() - 3);
        for (std::uint32_t column = 0; column < 16; ++column) {
            const auto start = 1U + column * 3U;
            if (start + 2U > cells.size()) {
                break;
            }

            const auto token = cells.substr(start, 2);
            const auto address = *row + column;
            if (address > 0x7f || token == "  " || token == "--") {
                continue;
            }

            if (token == "UU") {
                devices.push_back({
                    .address = static_cast<std::uint8_t>(address),
                    .claimedByKernel = true
                });
                continue;
            }

            const auto parsed = parseHex(token);
            if (parsed && *parsed == address) {
                devices.push_back({
                    .address = static_cast<std::uint8_t>(address),
                    .claimedByKernel = false
                });
            }
        }
    }

    return devices;
}

[[noreturn]] void throwCommandFailure(
    std::string_view command,
    const CommandResult& result
) {
    const auto detail = !result.standardError.empty()
        ? result.standardError
        : result.standardOutput;
    throw std::runtime_error(
        std::string(command) + " failed with exit code " +
        std::to_string(result.exitCode) +
        (detail.empty() ? std::string{} : ": " + trim(detail)));
}

} // namespace

I2cToolsProvider::I2cToolsProvider(
    std::shared_ptr<SystemReader> reader,
    std::vector<std::uint32_t> scanBuses
)
    : reader_(std::move(reader)),
      scanBuses_(scanBuses.begin(), scanBuses.end())
{
    if (!reader_) {
        throw std::invalid_argument("I2cToolsProvider requires a SystemReader");
    }
}

std::vector<I2cBusInfo> I2cToolsProvider::buses() {
    const auto command = std::string(kToolPath) + "i2cdetect -l";
    const auto result = reader_->execute(command);
    if (!result.succeeded()) {
        throwCommandFailure("i2cdetect -l", result);
    }

    std::vector<I2cBusInfo> resultBuses;
    std::istringstream input(result.standardOutput);
    std::string line;
    while (std::getline(input, line)) {
        if (auto bus = parseBus(std::move(line))) {
            resultBuses.push_back(std::move(*bus));
        }
    }
    return resultBuses;
}

std::optional<std::vector<I2cDeviceInfo>> I2cToolsProvider::devices(
    std::uint32_t bus
) {
    if (!scanBuses_.contains(bus)) {
        return std::nullopt;
    }

    const auto command = std::string(kToolPath) +
        "i2cdetect -y " + std::to_string(bus);
    const auto result = reader_->execute(command);
    if (!result.succeeded()) {
        throwCommandFailure(command, result);
    }
    return parseScan(result.standardOutput);
}

} // namespace system_monitor