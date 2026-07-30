#pragma once

#include "features/departments/data/portal_department_query.hpp"

#include <drogular/query_string_builder.hpp>

#include <string>

class PortalDepartmentQuerySerializer {
public:
    static std::string toQueryString(const PortalDepartmentQuery& query) {
        drogular::QueryStringBuilder builder;

        builder.add("search", query.search);

        if (query.isActive) {
            builder.add("active", *query.isActive ? "true" : "false");
        }

        if (!query.sorting.empty()) {
            builder
                .add("sort", query.sorting.front().field)
                .add("direction", toString(query.sorting.front().direction));
        }

        builder.addIf(
            query.page != 1,
            "page",
            query.page
        );
        builder.addIf(
            query.pageSize != 10,
            "pageSize",
            query.pageSize
        );

        return builder.build();
    }
};