#pragma once

#include "core/portal_string_utils.hpp"
#include "features/projects/data/portal_project.hpp"
#include "features/projects/data/portal_project_create.hpp"
#include "features/projects/data/portal_project_update.hpp"
#include "features/projects/providers/project_provider.hpp"

#include <drogular/pagination.hpp>

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

    drogular::PagedResult<PortalProject> search(
        const PortalProjectQuery& query
    ) const override {
        std::vector<PortalProject> result;

        const auto needle =
            query.search.has_value()
                ? portalAsciiLowercase(*query.search)
                : std::string();

        for (const auto& project : projects_) {
            if (!needle.empty() &&
                portalAsciiLowercase(project.title).find(needle) == std::string::npos) {
                continue;
            }

            if (query.status.has_value() &&
                project.status != *query.status) {
                continue;
            }

            if (query.projectTypeId.has_value() &&
                project.projectTypeId !=
                    *query.projectTypeId) {
                continue;
            }

            if (query.ownerId.has_value() &&
                project.ownerId != *query.ownerId) {
                continue;
            }

            result.push_back(project);
        }

        auto sorting = query.sorting;

        if (sorting.empty()) {
            sorting.push_back({
                .field = "title",
                .direction =
                    PortalSortDirection::Ascending
            });
        }

        const auto compare =
            []<typename T>(
                const T& left,
                const T& right,
                PortalSortDirection direction
            ) {
                return direction ==
                       PortalSortDirection::Ascending
                    ? left < right
                    : right < left;
            };

        std::stable_sort(
            result.begin(),
            result.end(),
            [&sorting, &compare](
                const PortalProject& left,
                const PortalProject& right
            ) {
                for (const auto& sort : sorting) {
                    if (sort.field == "title" &&
                        left.title != right.title) {
                        return compare(
                            left.title,
                            right.title,
                            sort.direction
                        );
                    }

                    if (sort.field == "status" &&
                        left.status != right.status) {
                        return compare(
                            left.status,
                            right.status,
                            sort.direction
                        );
                    }

                    if (sort.field == "id" &&
                        left.id != right.id) {
                        return compare(
                            left.id,
                            right.id,
                            sort.direction
                        );
                    }
                }

                return left.id < right.id;
            }
        );

        return drogular::paginate(
            result,
            query.page,
            query.pageSize
        );
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