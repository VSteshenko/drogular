#pragma once

#include "core/portal_request_parameters.hpp"
#include "features/projects/data/portal_project_query.hpp"

#include <algorithm>
#include <string>

class PortalProjectQueryParser {
public:
    static PortalProjectQuery fromRequest(
        const drogon::HttpRequestPtr& request
    ) {
        PortalProjectQuery query;
        const PortalRequestParameters parameters(request);

        query.search = parameters.optionalString("search");
        query.status = parameters.optionalString("status");
        query.projectTypeId = parameters.positiveInteger("projectTypeId");
        query.ownerId = parameters.positiveInteger("ownerId");
        query.page = parameters.positiveIntegerOr("page", 1);
        query.pageSize = parameters.boundedPositiveIntegerOr(
            "pageSize",
            10,
            100
        );
        query.sorting.push_back({
            .field = sortField(parameters.value("sort")),
            .direction = sortDirection(parameters.value("direction"))
        });

        return query;
    }

private:
    static std::string sortField(const std::string& value) {
        if (value == "id" || value == "status" || value == "title") {
            return value;
        }
        return "title";
    }

    static PortalSortDirection sortDirection(const std::string& value) {
        return value == "desc"
            ? PortalSortDirection::Descending
            : PortalSortDirection::Ascending;
    }
};