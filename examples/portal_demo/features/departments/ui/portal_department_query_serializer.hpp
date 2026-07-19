#pragma once

#include "features/departments/data/portal_department_query.hpp"

#include <drogular/url.hpp>

#include <string>
#include <vector>

class PortalDepartmentQuerySerializer {
public:
    static std::string toQueryString(
        const PortalDepartmentQuery& query
    ) {
        std::vector<std::string> values;

        if (query.search && !query.search->empty()) {
            values.push_back(
                "search=" + drogular::Url::encode(*query.search)
            );
        }

        if (query.isActive) {
            values.push_back(
                std::string("active=") + (*query.isActive ? "true" : "false")
            );
        }

        if (!query.sorting.empty()) {
            values.push_back(
                "sort=" + drogular::Url::encode(query.sorting.front().field)
            );
            values.push_back(
                "direction=" + toString(query.sorting.front().direction)
            );
        }

        if (query.page != 1) {
            values.push_back("page=" + std::to_string(query.page));
        }

        if (query.pageSize != 10) {
            values.push_back("pageSize=" + std::to_string(query.pageSize));
        }

        if (values.empty()) {
            return "";
        }

        std::string result = "?";
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (i) {
                result += "&";
            }

            result += values[i];
        }
        return result;
    }
};