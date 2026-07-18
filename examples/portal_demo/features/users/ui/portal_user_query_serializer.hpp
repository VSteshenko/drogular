#pragma once

#include "features/users/data/portal_user_query.hpp"

#include <drogular/url.hpp>

#include <string>

class PortalUserQuerySerializer {
public:
    static std::string toQueryString(
        const PortalUserQuery& query
    ) {
        std::string result;
        std::string separator = "?";

        const auto append =
            [&result, &separator](
                const std::string& name,
                const std::string& value
            ) {
                result += separator + name + "=" +
                    drogular::Url::encode(value);
                separator = "&";
            };

        if (query.search.has_value() &&
            !query.search->empty()) {
            append("search", *query.search);
        }

        if (query.role.has_value() &&
            !query.role->empty()) {
            append("role", *query.role);
        }

        const auto sort = effectiveSort(query);

        if (sort.field != "username" ||
            sort.direction !=
                PortalUserSortDirection::Ascending) {
            append("sort", sort.field);
            append("direction", toString(sort.direction));
        }

        if (query.pageSize != 10 && query.pageSize > 0) {
            append("pageSize", std::to_string(query.pageSize));
        }

        if (query.page > 1) {
            append("page", std::to_string(query.page));
        }

        return result;
    }

private:
    static PortalUserSort effectiveSort(
        const PortalUserQuery& query
    ) {
        if (!query.sorting.empty()) {
            return query.sorting.front();
        }

        return {
            .field = "username",
            .direction = PortalUserSortDirection::Ascending
        };
    }
};