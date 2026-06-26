#pragma once

#include "portal_project.hpp"

#include <string>
#include <utility>
#include <vector>

class PortalProjectRepository {
public:
    PortalProjectRepository()
        : projects_({
              {1, "Customer Portal", "active"},
              {2, "Internal Dashboard", "paused"}
          }),
          nextId_(3) {
    }

    std::vector<PortalProject> all() const {
        return projects_;
    }

    void create(
        std::string title,
        std::string status
    ) {
        projects_.push_back({
            .id = nextId_++,
            .title = std::move(title),
            .status = std::move(status)
        });
    }

private:
    std::vector<PortalProject> projects_;
    int nextId_ = 1;
};
