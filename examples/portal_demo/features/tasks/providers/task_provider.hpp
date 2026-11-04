#pragma once

#include "features/tasks/data/portal_task.hpp"

#include <optional>
#include <vector>

class PortalTaskProvider {
public:
    virtual ~PortalTaskProvider() = default;

    virtual std::vector<PortalTask> all() const = 0;
    virtual std::optional<PortalTask> findById(int id) const = 0;
};