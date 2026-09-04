#include "processes_fragment_component.hpp"
#include "services/process_service.hpp"
#include "ui/system_formatters.hpp"

#include <drogular/render_context.hpp>

#include <json/json.h>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace system_monitor {
namespace {

constexpr std::size_t processLimit = 50;

std::string lowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool containsQuery(const ProcessInfo& process, const std::string& query) {
    if (query.empty()) {
        return true;
    }

    const auto matches = [&](std::string value) {
        return lowerCopy(std::move(value)).find(query) != std::string::npos;
    };

    return matches(std::to_string(process.pid)) ||
           matches(process.user) ||
           matches(process.name) ||
           matches(process.command);
}

std::string requestParameter(
    const drogular::RenderContext& context,
    const std::string& name,
    std::string fallback = {}) {
    const auto request = context.request();
    if (request == nullptr) {
        return fallback;
    }

    const auto value = request->getParameter(name);
    return value.empty() ? std::move(fallback) : value;
}

std::string formatAge(
    drogular::RenderContext& context,
    std::chrono::milliseconds age) {
    const auto seconds = std::max<std::int64_t>(0, age.count()) / 1000;
    if (seconds < 60) {
        return std::to_string(seconds) + context.translate("client.second_short");
    }

    const auto minutes = seconds / 60;
    if (minutes < 60) {
        return std::to_string(minutes) + context.translate("client.minute_short");
    }

    return std::to_string(minutes / 60) + context.translate("client.hour_short");
}

std::string formatClock(std::chrono::system_clock::time_point time) {
    if (time.time_since_epoch() == std::chrono::system_clock::duration::zero()) {
        return {};
    }

    const auto value = std::chrono::system_clock::to_time_t(time);
    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &value);
#else
    localtime_r(&value, &local);
#endif

    char buffer[16]{};
    if (std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &local) == 0) {
        return {};
    }
    return buffer;
}

} // namespace

void ProcessesFragmentComponent::onInit(drogular::RenderContext& context) {
    const auto service = context.requireService<ProcessService>();
    const auto processes = service->snapshot();
    const auto statistics = service->statistics();

    auto query = lowerCopy(requestParameter(context, "query"));
    const auto sort = requestParameter(context, "sort", "cpu");

    std::vector<ProcessInfo> visible;
    visible.reserve(processes.size());
    for (const auto& process : processes) {
        if (containsQuery(process, query)) {
            visible.push_back(process);
        }
    }

    std::sort(visible.begin(), visible.end(), [&](const ProcessInfo& left, const ProcessInfo& right) {
        if (sort == "memory") {
            return left.memoryPercent > right.memoryPercent;
        }
        if (sort == "rss") {
            return left.residentBytes > right.residentBytes;
        }
        if (sort == "pid") {
            return left.pid < right.pid;
        }
        if (sort == "name") {
            return left.name < right.name;
        }
        return left.cpuPercent > right.cpuPercent;
    });

    const auto shownCount = std::min(processLimit, visible.size());
    Json::Value rows(Json::arrayValue);
    for (std::size_t index = 0; index < shownCount; ++index) {
        const auto& process = visible[index];
        Json::Value row(Json::objectValue);
        row["pid"] = Json::Int64(process.pid);
        row["user"] = process.user;
        row["name"] = process.name;
        row["command"] = process.command.empty() ? process.name : process.command;
        row["cpu"] = ui::formatPercent(process.cpuPercent);
        row["memory"] = ui::formatPercent(process.memoryPercent);
        row["rss"] = ui::formatBytes(process.residentBytes);
        rows.append(std::move(row));
    }

    const auto shown = std::to_string(shownCount);
    const auto total = std::to_string(processes.size());
    const auto matching = std::to_string(visible.size());
    if (!query.empty()) {
        context.set(
            "processSummary",
            shown + " / " + matching + " " + context.translate("client.matching") +
                " · " + total + " " + context.translate("client.total"));
    } else {
        context.set(
            "processSummary",
            shown + " / " + total + " " + context.translate("client.shown"));
    }

    if (!statistics.healthy) {
        context.set(
            "processStatus",
            context.translate("status.stale") + " · " +
                formatAge(context, statistics.snapshotAge) + " " +
                context.translate("client.old"));
    } else {
        const auto updated = formatClock(statistics.lastSuccessfulUpdate);
        context.set(
            "processStatus",
            updated.empty()
                ? context.translate("client.process_available")
                : context.translate("client.updated") + " " + updated);
    }

    context.set("hasProcesses", shownCount != 0);
    context.set("hasQuery", !query.empty());
    context.set("processRows", std::move(rows));
}

} // namespace system_monitor