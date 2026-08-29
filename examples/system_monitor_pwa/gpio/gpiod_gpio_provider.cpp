#include "gpiod_gpio_provider.hpp"
#include "gpio_model.hpp"

#include <charconv>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace system_monitor {

namespace {

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::optional<GpioChipInfo> parseChipLine(std::string line) {
    line = trim(std::move(line));
    if (!line.starts_with("gpiochip")) {
        return std::nullopt;
    }

    const auto labelBegin = line.find('[');
    const auto labelEnd = labelBegin == std::string::npos
        ? std::string::npos
        : line.find(']', labelBegin + 1);
    const auto countBegin = labelEnd == std::string::npos
        ? std::string::npos
        : line.find('(', labelEnd + 1);
    const auto countEnd = countBegin == std::string::npos
        ? std::string::npos
        : line.find(" lines)", countBegin + 1);

    if (labelBegin == std::string::npos ||
        labelEnd == std::string::npos ||
        countBegin == std::string::npos ||
        countEnd == std::string::npos) {
        return std::nullopt;
    }

    GpioChipInfo chip;
    chip.name = trim(line.substr(0, labelBegin));
    chip.label = line.substr(labelBegin + 1, labelEnd - labelBegin - 1);

    const auto countText = trim(
        line.substr(countBegin + 1, countEnd - countBegin - 1));
    std::uint32_t lineCount = 0;
    const auto parsed = std::from_chars(
        countText.data(),
        countText.data() + countText.size(),
        lineCount);
    if (parsed.ec != std::errc{} ||
        parsed.ptr != countText.data() + countText.size()) {
        return std::nullopt;
    }

    chip.lineCount = lineCount;
    return chip;
}

} // namespace

GpiodGpioProvider::GpiodGpioProvider(std::shared_ptr<SystemReader> reader)
    : reader_(std::move(reader))
{
    if (!reader_) {
        throw std::invalid_argument("GpiodGpioProvider requires a SystemReader");
    }
}

std::vector<GpioChipInfo> GpiodGpioProvider::chips() {
    const auto result = reader_->execute("gpiodetect");
    if (!result.succeeded()) {
        const auto detail = !result.standardError.empty()
            ? result.standardError
            : result.standardOutput;
        throw std::runtime_error(
            "gpiodetect failed with exit code " +
            std::to_string(result.exitCode) +
            (detail.empty() ? std::string{} : ": " + trim(detail)));
    }

    std::vector<GpioChipInfo> chips;
    std::istringstream output(result.standardOutput);
    std::string line;
    while (std::getline(output, line)) {
        if (auto chip = parseChipLine(std::move(line))) {
            chips.push_back(std::move(*chip));
        }
    }

    return chips;
}

} // namespace system_monitor