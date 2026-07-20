#pragma once

#include "core/portal_query_string_builder.hpp"
#include "features/users/data/portal_user_query.hpp"

#include <string>

class PortalUserQuerySerializer {
public:
    static std::string toQueryString(const PortalUserQuery& query) {
        PortalQueryStringBuilder builder;

        if (query.search && !query.search->empty()) {
            builder.add("search", *query.search);
        }
        if (query.role && !query.role->empty()) {
            builder.add("role", *query.role);
        }

        const auto sort = effectiveSort(query);
        if (sort.field != "username" || sort.direction != PortalSortDirection::Ascending) {
            builder
                .add("sort", sort.field)
                .add("direction", toString(sort.direction));
        }

        builder.addIf(
            query.pageSize != 10 && query.pageSize > 0,
            "pageSize",
            query.pageSize
        );
        builder.addIf(
            query.page > 1,
            "page",
            query.page
        );
        return builder.build();
    }

private:
    static PortalUserSort effectiveSort(const PortalUserQuery& query) {
        return query.sorting.empty()
            ? PortalUserSort{
                .field = "username",
                .direction = PortalSortDirection::Ascending
            }
            : query.sorting.front();
    }
};