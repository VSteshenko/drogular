#pragma once

#include "features/projects/data/portal_project_query.hpp"

#include <drogular/query_string_builder.hpp>

#include <string>

class PortalProjectQuerySerializer {
public:
    static std::string toQueryString(const PortalProjectQuery& query) {
        drogular::QueryStringBuilder builder;

        builder.add("search", query.search);

        builder.add("status", query.status);

        if (query.projectTypeId && *query.projectTypeId > 0) {
            builder.add("projectTypeId", *query.projectTypeId);
        }

        if (query.ownerId && *query.ownerId > 0) {
            builder.add("ownerId", *query.ownerId);
        }

        const auto sort = effectiveSort(query);
        if (sort.field != "title" || sort.direction != PortalSortDirection::Ascending) {
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
    static PortalProjectSort effectiveSort(const PortalProjectQuery& query) {
        return query.sorting.empty()
            ? PortalProjectSort{
                .field = "title",
                .direction = PortalSortDirection::Ascending
            }
            : query.sorting.front();
    }
};