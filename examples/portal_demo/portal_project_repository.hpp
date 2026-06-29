#pragma once

#include "portal_project.hpp"

#include <string>
#include <utility>
#include <vector>
#include <optional>

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

    std::optional<PortalProject> findById(
        int id
    ) const {
        for (const auto& project : projects_) {
            if (project.id == id) {
                return project;
            }
        }

        return std::nullopt;
    }

    bool update(
        int id,
        std::string title,
        std::string status
    ) {
        for (auto& project : projects_) {
            if (project.id == id) {
                project.title = std::move(title);
                project.status = std::move(status);
                return true;
            }
        }

        return false;
    }

private:
    std::vector<PortalProject> projects_;
    int nextId_ = 1;
};
