#pragma once

#include <string>

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