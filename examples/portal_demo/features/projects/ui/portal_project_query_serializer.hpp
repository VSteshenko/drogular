#pragma once

#include "features/projects/data/portal_project_query.hpp"

#include <drogular/url.hpp>

#include <string>

class PortalProjectQuerySerializer {
public:
    static std::string toQueryString(
        const PortalProjectQuery& query
    ) {
        std::string result;
        std::string separator = "?";

        const auto append =
            [&result, &separator](
                const std::string& name,
                const std::string& value
            ) {
                result +=
                    separator +
                    name +
                    "=" +
                    drogular::Url::encode(value);
                separator = "&";
            };

        if (query.search.has_value() &&
            !query.search->empty()) {
            append("search", *query.search);
        }

        if (query.status.has_value() &&
            !query.status->empty()) {
            append("status", *query.status);
        }

        if (query.projectTypeId.has_value() &&
            *query.projectTypeId > 0) {
            append(
                "projectTypeId",
                std::to_string(*query.projectTypeId)
            );
        }

        if (query.ownerId.has_value() &&
            *query.ownerId > 0) {
            append(
                "ownerId",
                std::to_string(*query.ownerId)
            );
        }

        const auto sort = effectiveSort(query);

        if (sort.field != "title" ||
            sort.direction !=
                PortalProjectSortDirection::Ascending) {
            append("sort", sort.field);
            append("direction", toString(sort.direction));
        }

        if (query.page > 1) {
            append("page", std::to_string(query.page));
        }

        return result;
    }

private:
    static PortalProjectSort effectiveSort(
        const PortalProjectQuery& query
    ) {
        if (!query.sorting.empty()) {
            return query.sorting.front();
        }

        return {
            .field = "title",
            .direction =
                PortalProjectSortDirection::Ascending
        };
    }
};