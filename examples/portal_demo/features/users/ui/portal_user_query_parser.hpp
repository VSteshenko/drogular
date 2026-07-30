#pragma once

#include "features/users/data/portal_user_query.hpp"

#include <drogular/request_parameters.hpp>

#include <string>

class PortalUserQueryParser {
public:
    static PortalUserQuery fromRequest(
        const drogon::HttpRequestPtr& request
    ) {
        PortalUserQuery query;
        const drogular::RequestParameters parameters(request);

        query.search = parameters.optionalString("search");
        query.role = parameters.optionalString("role");
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
        if (value == "id" || value == "role" || value == "username") {
            return value;
        }
        return "username";
    }

    static PortalSortDirection sortDirection(const std::string& value) {
        return value == "desc"
            ? PortalSortDirection::Descending
            : PortalSortDirection::Ascending;
    }
};