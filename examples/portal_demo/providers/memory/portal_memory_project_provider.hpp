#pragma once

#include "../../data/models/portal_project.hpp"
#include "../../data/models/portal_project_create.hpp"
#include "../../data/models/portal_project_update.hpp"
#include "../project_provider.hpp"

#include <string>
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
        const PortalProjectCreate& input,
        int ownerId
    ) override {
        PortalProject project;

        project.id = nextId_++;
        project.title = input.title;
        project.status = input.status.value_or("active");
        project.ownerId = ownerId;
        project.projectTypeId = input.projectTypeId;

        projects_.push_back(project);

        return project;
    }

    PortalProject update(
        const PortalProjectUpdate& input
    ) override {
        for (auto& project : projects_) {
            if (project.id != input.id) {
                continue;
            }

            if (input.title.has_value()) {
                project.title =
                    *input.title;
            }

            if (input.status.has_value()) {
                project.status =
                    *input.status;
            }

            if (input.projectTypeId.has_value()) {
                project.projectTypeId =
                    *input.projectTypeId;
            }

            return project;
        }

        return {};
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