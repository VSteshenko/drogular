#pragma once

#include "features/departments/data/portal_department_query.hpp"

#include <drogon/HttpRequest.h>

#include <algorithm>
#include <cstdlib>
#include <memory>

class PortalDepartmentQueryParser {
public:
    static PortalDepartmentQuery fromRequest(
        const drogon::HttpRequestPtr& request
    ) {
        PortalDepartmentQuery query;

        if (!request) {
            return query;
        }

        const auto search =
            request->getParameter("search");
        if (!search.empty()) {
            query.search = search;
        }

        const auto active =
            request->getParameter("active");
        if (active == "true") {
            query.isActive = true;
        } else if (active == "false") {
            query.isActive = false;
        }

        const auto sort =
            request->getParameter("sort");
        const auto direction =
            request->getParameter("direction");
        if (sort == "id" || sort == "name" || sort == "managerId" || sort == "isActive") {
            query.sorting.push_back({
                sort,
                direction == "desc"
                    ? PortalDepartmentSortDirection::Descending
                    : PortalDepartmentSortDirection::Ascending
            });
        }

        query.page = positive(
            request->getParameter("page"),
            1
        );
        query.pageSize =
            std::min(100,
                positive(
                    request->getParameter("pageSize"),
                    10
                )
            );
        return query;
    }

private:
    static int positive(
        const std::string& value,
        int fallback
    ) {
        if (value.empty()) {
            return fallback;
        }

        char* end = nullptr;
        const auto result =
            std::strtol(value.c_str(), &end, 10);
        return end != value.c_str()
            && *end == '\0'
            && result > 0
                ? static_cast<int>(result)
                : fallback;
    }
};