#pragma once

#include "features/departments/data/portal_department_query.hpp"

#include <drogular/request_parameters.hpp>

#include <string>

class PortalDepartmentQueryParser {
public:
    static PortalDepartmentQuery fromRequest(
        const drogon::HttpRequestPtr& request
    ) {
        PortalDepartmentQuery query;
        const drogular::RequestParameters parameters(request);

        query.search = parameters.optionalString("search");

        const auto active = parameters.value("active");
        if (active == "true") {
            query.isActive = true;
        } else if (active == "false") {
            query.isActive = false;
        }

        const auto sort = parameters.value("sort");
        if (sort == "id" || sort == "name" ||
            sort == "managerId" || sort == "isActive") {
            query.sorting.push_back({
                sort,
                parameters.value("direction") == "desc"
                    ? PortalSortDirection::Descending
                    : PortalSortDirection::Ascending
            });
        }

        query.page = parameters.positiveIntegerOr(
            "page",
            1
        );
        query.pageSize = parameters.boundedPositiveIntegerOr(
            "pageSize",
            10,
            100
        );
        return query;
    }
};