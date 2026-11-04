#pragma once

#include "features/tasks/providers/task_provider.hpp"

#include <utility>

class PortalMemoryTaskProvider final : public PortalTaskProvider {
public:
    PortalMemoryTaskProvider()
        : PortalMemoryTaskProvider({
            {
                1,
                "Review portal permissions",
                PortalTaskStatus::InProgress,
                PortalTaskPriority::High,
                1
            },
            {
                2,
                "Verify dashboard metrics",
                PortalTaskStatus::Todo,
                PortalTaskPriority::Normal,
                2
            },
            {
                3,
                "Update onboarding checklist",
                PortalTaskStatus::Blocked,
                PortalTaskPriority::Critical,
                1
            },
            {
                4,
                "Document API contracts",
                PortalTaskStatus::Done,
                PortalTaskPriority::Low,
                2
            }
        }) {}

    explicit PortalMemoryTaskProvider(std::vector<PortalTask> tasks)
        : tasks_(std::move(tasks)) {}

    std::vector<PortalTask> all() const override {
        return tasks_;
    }

    std::optional<PortalTask> findById(int id) const override {
        for (const auto& task : tasks_) {
            if (task.id == id) return task;
        }
        return std::nullopt;
    }

private:
    std::vector<PortalTask> tasks_;
};