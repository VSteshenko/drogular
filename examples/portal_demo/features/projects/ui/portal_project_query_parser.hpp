#pragma once

#include "features/projects/data/portal_project_query.hpp"

#include <drogon/HttpRequest.h>

#include <algorithm>
#include <exception>
#include <optional>
#include <string>

class PortalProjectQueryParser {
public:
    static PortalProjectQuery fromRequest(
        const drogon::HttpRequestPtr& request
    ) {
        PortalProjectQuery query;

        if (request == nullptr) {
            query.sorting.push_back(defaultSort());
            return query;
        }

        query.search = optionalString(
            request->getParameter("search")
        );
        query.status = optionalString(
            request->getParameter("status")
        );
        query.projectTypeId = positiveInteger(
            request->getParameter("projectTypeId")
        );
        query.ownerId = positiveInteger(
            request->getParameter("ownerId")
        );
        query.page = positiveIntegerOrDefault(
            request->getParameter("page"),
            1
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
    static PortalProjectSort defaultSort() {
        return {
            .field = "title",
            .direction =
                PortalProjectSortDirection::Ascending
        };
    }

    static std::optional<std::string> optionalString(
        const std::string& value
    ) {
        if (value.empty()) {
            return std::nullopt;
        }

        return value;
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
        const auto parsed = positiveInteger(value);
        return parsed.value_or(defaultValue);
    }

    static std::string sortField(
        const std::string& value
    ) {
        if (value == "id" ||
            value == "status" ||
            value == "title") {
            return value;
        }

        return "title";
    }

    static PortalProjectSortDirection sortDirection(
        const std::string& value
    ) {
        return value == "desc"
            ? PortalProjectSortDirection::Descending
            : PortalProjectSortDirection::Ascending;
    }
};