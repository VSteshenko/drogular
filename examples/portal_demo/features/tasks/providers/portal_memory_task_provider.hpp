#pragma once

#include "features/tasks/providers/task_provider.hpp"
#include "data/demo_dataset.hpp"

#include <utility>
#include <mutex>

class PortalMemoryTaskProvider final : public PortalTaskProvider {
public:
    PortalMemoryTaskProvider()
        : PortalMemoryTaskProvider(DemoDataset::create().tasks())
    {
    }

    explicit PortalMemoryTaskProvider(std::vector<PortalTask> tasks)
        : tasks_(std::move(tasks)) {}

    std::vector<PortalTask> all() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return tasks_;
    }

    std::optional<PortalTask> findById(int id) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& task : tasks_) {
            if (task.id == id) return task;
        }
        return std::nullopt;
    }

    bool updateStatus(int id, PortalTaskStatus status) override {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& task : tasks_) {
            if (task.id == id) {
                task.status = status;
                return true;
            }
        }
        return false;
    }

    bool updatePriority(int id, PortalTaskPriority priority) override {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& task : tasks_) {
            if (task.id == id) {
                task.priority = priority;
                return true;
            }
        }
        return false;
    }

private:
    mutable std::mutex mutex_;
    std::vector<PortalTask> tasks_;
};