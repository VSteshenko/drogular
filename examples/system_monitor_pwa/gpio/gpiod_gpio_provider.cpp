#include "gpiod_gpio_provider.hpp"

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

std::string shellQuote(std::string_view value) {
    std::string result("'");
    for (const char ch : value) {
        if (ch == '\'') {
            result += "'\\''";
        } else {
            result += ch;
        }
    }
    result += '\'';
    return result;
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

std::vector<std::string> quotedValues(std::string_view text) {
    std::vector<std::string> values;
    std::size_t position = 0;

    while (position < text.size()) {
        const auto begin = text.find('"', position);
        if (begin == std::string_view::npos) {
            break;
        }

        std::string value;
        std::size_t cursor = begin + 1;
        bool closed = false;
        while (cursor < text.size()) {
            if (text[cursor] == '\\' && cursor + 1 < text.size()) {
                value += text[cursor + 1];
                cursor += 2;
                continue;
            }
            if (text[cursor] == '"') {
                closed = true;
                break;
            }
            value += text[cursor++];
        }

        if (!closed) {
            break;
        }

        values.push_back(std::move(value));
        position = cursor + 1;
    }

    return values;
}

bool isTokenBoundary(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

bool containsToken(std::string_view text, std::string_view token) {
    std::size_t position = 0;
    while ((position = text.find(token, position)) != std::string_view::npos) {
        const bool leftBoundary =
            position == 0 || isTokenBoundary(text[position - 1]);
        const auto after = position + token.size();
        const bool rightBoundary =
            after == text.size() || isTokenBoundary(text[after]);

        if (leftBoundary && rightBoundary) {
            return true;
        }
        position = after;
    }

    return false;
}

std::optional<std::string_view> attributeValue(
    std::string_view text,
    std::string_view name
) {
    const std::string key = std::string(name) + "=";
    auto position = text.find(key);

    while (position != std::string_view::npos) {
        if (position == 0 || isTokenBoundary(text[position - 1])) {
            auto value = text.substr(position + key.size());
            if (value.empty()) {
                return std::string_view{};
            }

            if (value.front() == '"') {
                return std::nullopt;
            }

            const auto end = value.find_first_of(" \t\r\n");
            return value.substr(0, end);
        }

        position = text.find(key, position + key.size());
    }

    return std::nullopt;
}

std::optional<std::string> consumerValue(std::string_view text) {
    constexpr std::string_view marker = "consumer=";
    const auto position = text.find(marker);
    if (position == std::string_view::npos) {
        return std::nullopt;
    }

    auto value = text.substr(position + marker.size());
    if (value.empty()) {
        return std::string{};
    }

    if (value.front() == '"') {
        std::string result;
        for (std::size_t i = 1; i < value.size(); ++i) {
            if (value[i] == '\\' && i + 1 < value.size()) {
                result += value[++i];
            } else if (value[i] == '"') {
                return result;
            } else {
                result += value[i];
            }
        }
        return std::nullopt;
    }

    const auto end = value.find_first_of(" \t\r\n");
    return std::string(value.substr(0, end));
}

std::optional<GpioLineInfo> parseLineInfo(std::string line) {
    line = trim(std::move(line));
    if (!line.starts_with("line")) {
        return std::nullopt;
    }

    const auto colon = line.find(':');
    if (colon == std::string::npos) {
        return std::nullopt;
    }

    auto offsetText = trim(line.substr(4, colon - 4));
    std::uint32_t offset = 0;
    const auto parsed = std::from_chars(
        offsetText.data(),
        offsetText.data() + offsetText.size(),
        offset);
    if (parsed.ec != std::errc{} ||
        parsed.ptr != offsetText.data() + offsetText.size()) {
        return std::nullopt;
    }

    const std::string_view details(line.data() + colon + 1, line.size() - colon - 1);
    const auto quoted = quotedValues(details);

    GpioLineInfo info;
    info.offset = offset;

    if (!quoted.empty()) {
        info.name = quoted.front();
    } else {
        const auto first = trim(std::string(details));
        if (!first.starts_with("unnamed")) {
            return std::nullopt;
        }
    }

    if (containsToken(details, "output")) {
        info.direction = GpioLineDirection::Output;
    } else if (containsToken(details, "input")) {
        info.direction = GpioLineDirection::Input;
    }

    info.activeLow = containsToken(details, "active-low");

    if (const auto drive = attributeValue(details, "drive")) {
        if (*drive == "open-drain") {
            info.drive = GpioLineDrive::OpenDrain;
        } else if (*drive == "open-source") {
            info.drive = GpioLineDrive::OpenSource;
        } else if (*drive == "push-pull") {
            info.drive = GpioLineDrive::PushPull;
        } else {
            info.drive = GpioLineDrive::Unknown;
        }
    }

    if (const auto consumer = consumerValue(details)) {
        info.consumer = *consumer;
        info.used = !consumer->empty();
    } else {
        // libgpiod 1.x prints the consumer as the second quoted field and
        // marks requested lines with "[used]".
        if (quoted.size() >= 2) {
            info.consumer = quoted[1];
        }
        info.used =
            containsToken(details, "[used]") ||
            (!info.consumer.empty() && !containsToken(details, "unused"));
    }

    return info;
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
        throwCommandFailure("gpiodetect", result);
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

std::vector<GpioLineInfo> GpiodGpioProvider::lines(std::string_view chip) {
    if (chip.empty()) {
        throw std::invalid_argument("GPIO chip must not be empty");
    }

    // libgpiod 2.x uses -c/--chip. libgpiod 1.x expects the chip as a
    // positional argument, so fall back only when the modern form fails.
    const auto quotedChip = shellQuote(chip);
    auto result = reader_->execute("gpioinfo -c " + quotedChip);
    if (!result.succeeded()) {
        result = reader_->execute("gpioinfo " + quotedChip);
    }
    if (!result.succeeded()) {
        throwCommandFailure("gpioinfo", result);
    }

    std::vector<GpioLineInfo> lines;
    std::istringstream output(result.standardOutput);
    std::string line;
    while (std::getline(output, line)) {
        if (auto info = parseLineInfo(std::move(line))) {
            lines.push_back(std::move(*info));
        }
    }

    return lines;
}

} // namespace system_monitor