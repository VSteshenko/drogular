#include "process_list_parser.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

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

std::uint64_t kibToBytes(std::uint64_t value) {
    constexpr std::uint64_t scale = 1024;
    if (value > std::numeric_limits<std::uint64_t>::max() / scale) {
        return std::numeric_limits<std::uint64_t>::max();
    }

    return value * scale;
}

} // namespace

std::vector<ProcessInfo> parseProcessList(std::string_view input) {
    std::vector<ProcessInfo> result;
    std::istringstream stream{std::string(input)};
    std::string line;

    while (std::getline(stream, line)) {
        if (trim(line).empty()) {
            continue;
        }

        std::istringstream row(line);
        ProcessInfo process;
        std::uint64_t residentKiB = 0;
        if (!(row >> process.pid >> process.user >> process.cpuPercent >>
              process.memoryPercent >> residentKiB >> process.name)) {
            continue;
        }

        std::string command;
        std::getline(row, command);
        process.command = trim(std::move(command));
        if (process.command.empty()) {
            process.command = process.name;
        }

        process.cpuPercent = std::isfinite(process.cpuPercent)
            ? std::max(0.0, process.cpuPercent)
            : 0.0;
        process.memoryPercent = std::isfinite(process.memoryPercent)
            ? std::max(0.0, process.memoryPercent)
            : 0.0;
        process.residentBytes = kibToBytes(residentKiB);
        result.push_back(std::move(process));
    }

    return result;
}

} // namespace system_monitor