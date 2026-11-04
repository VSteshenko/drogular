#pragma once

#include "features/tasks/providers/task_provider.hpp"
#include "data/demo_dataset.hpp"

#include <utility>

class PortalMemoryTaskProvider final : public PortalTaskProvider {
public:
    PortalMemoryTaskProvider()
        : PortalMemoryTaskProvider(DemoDataset::create().tasks())
    {
    }

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