#pragma once

#include "features/users/data/portal_user_query.hpp"

#include <drogon/HttpRequest.h>

#include <algorithm>
#include <exception>
#include <optional>
#include <string>

class PortalUserQueryParser {
public:
    static PortalUserQuery fromRequest(
        const drogon::HttpRequestPtr& request
    ) {
        PortalUserQuery query;

        if (request == nullptr) {
            query.sorting.push_back(defaultSort());
            return query;
        }

        query.search = optionalString(
            request->getParameter("search")
        );
        query.role = optionalString(
            request->getParameter("role")
        );
        query.page = positiveIntegerOrDefault(
            request->getParameter("page"),
            1
        );
        query.pageSize = positiveIntegerOrDefault(
            request->getParameter("pageSize"),
            10
        );
        query.sorting.push_back({
            .field = sortField(
                request->getParameter("sort")
            ),
            .direction = sortDirection(
                request->getParameter("direction")
            )
        });

        return query;
    }

private:
    static PortalUserSort defaultSort() {
        return {
            .field = "username",
            .direction =
                PortalUserSortDirection::Ascending
        };
    }

    static std::optional<std::string> optionalString(
        const std::string& value
    ) {
        return value.empty()
            ? std::nullopt
            : std::optional<std::string>(value);
    }

    static std::optional<int> positiveInteger(
        const std::string& value
    ) {
        if (value.empty()) {
            return std::nullopt;
        }

        try {
            const auto parsed = std::stoi(value);
            if (parsed > 0) {
                return parsed;
            }
        } catch (const std::exception&) {
        }

        return std::nullopt;
    }

    static int positiveIntegerOrDefault(
        const std::string& value,
        int defaultValue
    ) {
        return positiveInteger(value).value_or(defaultValue);
    }

    static std::string sortField(
        const std::string& value
    ) {
        if (value == "id" ||
            value == "role" ||
            value == "username") {
            return value;
        }

        return "username";
    }

    static PortalUserSortDirection sortDirection(
        const std::string& value
    ) {
        return value == "desc"
            ? PortalUserSortDirection::Descending
            : PortalUserSortDirection::Ascending;
    }
};