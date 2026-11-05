#pragma once

#include <string>
#include <optional>
#include <string_view>

// Task lifecycle is independent from the status of its optional parent project.
enum class PortalTaskStatus {
    Todo,
    InProgress,
    Blocked,
    Done
};

enum class PortalTaskPriority {
    Low,
    Normal,
    High,
    Critical
};

inline const char* portalTaskStatusKey(PortalTaskStatus status) {
    switch (status) {
    case PortalTaskStatus::Todo:
        return "dashboard.task.status.todo";
    case PortalTaskStatus::InProgress:
        return "dashboard.task.status.in_progress";
    case PortalTaskStatus::Blocked:
        return "dashboard.task.status.blocked";
    case PortalTaskStatus::Done:
        return "dashboard.task.status.done";
    }
    return "dashboard.task.status.todo";
}

inline const char* portalTaskPriorityKey(PortalTaskPriority priority) {
    switch (priority) {
    case PortalTaskPriority::Low:
        return "dashboard.task.priority.low";
    case PortalTaskPriority::Normal:
        return "dashboard.task.priority.normal";
    case PortalTaskPriority::High:
        return "dashboard.task.priority.high";
    case PortalTaskPriority::Critical:
        return "dashboard.task.priority.critical";
    }
    return "dashboard.task.priority.normal";
}

struct PortalTask {
    int id = 0;
    std::string title;
    PortalTaskStatus status = PortalTaskStatus::Todo;
    PortalTaskPriority priority = PortalTaskPriority::Normal;
    // Zero means no linked project. Tasks remain first-class domain objects.
    int projectId = 0;
};

inline std::optional<PortalTaskStatus> parsePortalTaskStatus(std::string_view value) {
    if (value == "todo") {
        return PortalTaskStatus::Todo;
    }
    if (value == "in_progress") {
        return PortalTaskStatus::InProgress;
    }
    if (value == "blocked") {
        return PortalTaskStatus::Blocked;
    }
    if (value == "done") {
        return PortalTaskStatus::Done;
    }
    return std::nullopt;
}

inline std::optional<PortalTaskPriority> parsePortalTaskPriority(std::string_view value) {
    if (value == "low") {
        return PortalTaskPriority::Low;
    }
    if (value == "normal") {
        return PortalTaskPriority::Normal;
    }
    if (value == "high") {
        return PortalTaskPriority::High;
    }
    if (value == "critical") {
        return PortalTaskPriority::Critical;
    }
    return std::nullopt;
}

inline const char* portalTaskStatusValue(PortalTaskStatus value) {
    switch (value) {
    case PortalTaskStatus::Todo:
        return "todo";
    case PortalTaskStatus::InProgress:
        return "in_progress";
    case PortalTaskStatus::Blocked:
        return "blocked";
    case PortalTaskStatus::Done:
        return "done";
    }
    return "todo";
}

inline const char* portalTaskPriorityValue(PortalTaskPriority value) {
    switch (value) {
    case PortalTaskPriority::Low:
        return "low";
    case PortalTaskPriority::Normal:
        return "normal";
    case PortalTaskPriority::High:
        return "high";
    case PortalTaskPriority::Critical:
        return "critical";
    }
    return "normal";
}