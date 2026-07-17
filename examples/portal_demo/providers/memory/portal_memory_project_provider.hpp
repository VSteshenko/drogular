#pragma once

#include "../../data/models/portal_project.hpp"
#include "../../data/models/portal_project_create.hpp"
#include "../../data/models/portal_project_update.hpp"
#include "../project_provider.hpp"

#include <string>
#include <vector>
#include <optional>
#include <algorithm>
#include <cstddef>
#include <algorithm>
#include <cctype>
#include <string>

static std::string lower(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char character) {
            return static_cast<char>(
                std::tolower(character)
            );
        }
    );

    return value;
}

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

    PortalPage<PortalProject> search(
        const PortalProjectQuery& query
    ) const override {
        std::vector<PortalProject> result;

        const auto needle =
            query.search.has_value()
                ? lower(*query.search)
                : std::string();

        for (const auto& project : projects_) {
            if (!needle.empty() &&
                !lower(project.title).contains(needle)) {
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
                    PortalProjectSortDirection::Ascending
            });
        }

        const auto compare =
            []<typename T>(
                const T& left,
                const T& right,
                PortalProjectSortDirection direction
            ) {
                return direction ==
                       PortalProjectSortDirection::Ascending
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

        PortalPage<PortalProject> page;
        page.page = std::max(1, query.page);
        page.pageSize = std::max(1, query.pageSize);
        page.totalItems = static_cast<int>(result.size());
        page.totalPages = std::max(
            1,
            (page.totalItems + page.pageSize - 1) /
                page.pageSize
        );

        const auto beginIndex =
            static_cast<std::size_t>(page.page - 1) *
            static_cast<std::size_t>(page.pageSize);

        if (beginIndex >= result.size()) {
            return page;
        }

        const auto endIndex =
            std::min(
                result.size(),
                beginIndex +
                    static_cast<std::size_t>(page.pageSize)
            );

        page.items.assign(
            result.begin() +
                static_cast<std::ptrdiff_t>(beginIndex),
            result.begin() +
                static_cast<std::ptrdiff_t>(endIndex)
        );

        return page;
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