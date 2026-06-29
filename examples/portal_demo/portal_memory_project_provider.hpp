#pragma once

#include "portal_project.hpp"
#include "portal_project_provider.hpp"

#include <string>
#include <utility>
#include <vector>
#include <optional>
#include <algorithm>

class PortalMemoryProjectProvider final
    : public PortalProjectProvider
{
public:
    PortalMemoryProjectProvider()
        : projects_({
              {1, "Customer Portal", "active"},
              {2, "Internal Dashboard", "paused"}
          }),
          nextId_(3) {
    }

    std::vector<PortalProject> all() const override {
        return projects_;
    }

    PortalProject create(
        PortalProject project
    ) override {
        project.id = nextId_++;
        projects_.push_back(project);
        return project;
    }

    std::optional<PortalProject> findById(
        int id
    ) const override {
        for (const auto& project : projects_) {
            if (project.id == id) {
                return project;
            }
        }

        return std::nullopt;
    }

    bool update(
        PortalProject project
    ) override {
        for (auto& existing : projects_) {
            if (existing.id == project.id) {
                existing = std::move(project);
                return true;
            }
        }

        return false;
    }

    bool remove(
        int id
    ) override {
        const auto originalSize =
            projects_.size();

        projects_.erase(
            std::remove_if(
                projects_.begin(),
                projects_.end(),
                [id](const PortalProject& project) {
                    return project.id == id;
                }
            ),
            projects_.end()
        );

        return projects_.size() != originalSize;
    }

private:
    std::vector<PortalProject> projects_;
    int nextId_ = 1;
};
