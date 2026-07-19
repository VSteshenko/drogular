#pragma once

#include "department_provider.hpp"

#include <algorithm>
#include <cstddef>

class PortalMemoryDepartmentProvider final
    : public PortalDepartmentProvider
{
public:
    PortalMemoryDepartmentProvider() : departments_({
        {
            1,
            "Engineering",
            "Product and platform engineering",
            1,
            true
        },
        {
            2,
            "Operations",
            "Internal operations and delivery",
            2,
            true
        },
        {
            3,
            "Archive",
            "Inactive historical department",
            1,
            false
        }
    }), nextId_(4) {
    }

    explicit PortalMemoryDepartmentProvider(
        std::vector<PortalDepartment> departments
    )
        : departments_(std::move(departments))
    {
        for (const auto& item : departments_) {
            nextId_ = std::max(nextId_, item.id + 1);
        }
    }

    std::vector<PortalDepartment> all() const override {
        return departments_;
    }

    std::optional<PortalDepartment> findById(
        int id
    ) const override {
        for (const auto& item : departments_) {
            if (item.id == id) {
                return item;
            }
        }

        return std::nullopt;
    }

    bool exists(
        const std::string& name,
        std::optional<int> excludingId = std::nullopt
    ) const override {
        const auto normalized =
            lowercase(name);

        for (const auto& item : departments_) {
            if (excludingId && item.id == *excludingId) {
                continue;
            }
            if (lowercase(item.name) == normalized) {
                return true;
            }
        }

        return false;
    }

    PortalDepartment create(
        const PortalDepartmentCreate& input
    ) override {
        PortalDepartment value{
            nextId_++,
            input.name,
            input.description,
            input.managerId,
            input.isActive
        };

        departments_.push_back(value);

        return value;
    }

    PortalDepartment update(
        const PortalDepartmentUpdate& input
    ) override {
        for (auto& value : departments_) {
            if (value.id == input.id) {
                if (input.name) {
                    value.name = *input.name;
                }
                if (input.description) {
                    value.description = *input.description;
                }
                if (input.managerId) {
                    value.managerId = *input.managerId;
                }
                if (input.isActive) {
                    value.isActive = *input.isActive;
                }

                return value;
            }
        }

        return {};
    }

    PortalPage<PortalDepartment> search(
        const PortalDepartmentQuery& query
    ) const override {
        std::vector<PortalDepartment> result;

        const auto needle =
            query.search
            ? lowercase(*query.search)
            : std::string();

        for (const auto& item : departments_) {
            if (!needle.empty() &&
                lowercase(item.name + " " + item.description)
                    .find(needle) == std::string::npos) {
                continue;
            }
            if (query.isActive && item.isActive != *query.isActive) {
                continue;
            }

            result.push_back(item);
        }

        auto sorting =
            query.sorting;
        if (sorting.empty()) {
            sorting.push_back({
                "name",
                PortalDepartmentSortDirection::Ascending
            });
        }

        std::stable_sort(
            result.begin(),
            result.end(),
            [&sorting](
                const auto& a,
                const auto& b
            ) {
                for (const auto& sort : sorting) {
                    int comparison = 0;

                    if (sort.field == "name") {
                        comparison = a.name.compare(b.name);
                    } else if (sort.field == "managerId") {
                        comparison = (a.managerId > b.managerId) - (a.managerId < b.managerId);
                    } else if (sort.field == "isActive") {
                        comparison = (a.isActive > b.isActive) - (a.isActive < b.isActive);
                    } else if (sort.field == "id") {
                        comparison = (a.id > b.id) - (a.id < b.id);
                    }

                    if (comparison != 0) {
                        return sort.direction == PortalDepartmentSortDirection::Ascending
                            ? comparison < 0
                            : comparison > 0;
                    }
                }
            return a.id < b.id;
        });

        PortalPage<PortalDepartment> page;

        page.page = std::max(1, query.page);
        page.pageSize = std::max(1, query.pageSize);
        page.totalItems = static_cast<int>(result.size());
        page.totalPages = std::max(1, (page.totalItems + page.pageSize - 1) / page.pageSize);

        const auto begin =
            static_cast<std::size_t>(page.page - 1) * static_cast<std::size_t>(page.pageSize);
        if (begin < result.size()) {
            const auto end =
                std::min(result.size(), begin + static_cast<std::size_t>(page.pageSize));

            page.items.assign(
                result.begin() + static_cast<std::ptrdiff_t>(begin),
                result.begin() + static_cast<std::ptrdiff_t>(end));
        }

        return page;
    }

private:
    static std::string lowercase(
        std::string value
    ) {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });

        return value;
    }

    std::vector<PortalDepartment> departments_;
    int nextId_ = 1;
};