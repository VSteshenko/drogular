#pragma once

#include "todo_query.hpp"

#include <drogon/HttpRequest.h>

#include <algorithm>
#include <charconv>
#include <string>
#include <system_error>

class TodoQueryParser {
public:
    static TodoQuery fromRequest(const drogon::HttpRequestPtr& request) {
        TodoQuery query;

        if (!request) {
            return query;
        }

        query.search =
            request->getParameter("search");
        query.page =
            positiveIntegerOr(request->getParameter("page"), 1);
        query.pageSize = std::min(
            positiveIntegerOr(request->getParameter("pageSize"), 5),
            20
        );

        return query;
    }

private:
    static int positiveIntegerOr(const std::string& value, int fallback) {
        if (value.empty()) {
            return fallback;
        }

        int result = 0;
        const auto parsed = std::from_chars(
            value.data(),
            value.data() + value.size(),
            result
        );

        if (parsed.ec != std::errc{} ||
            parsed.ptr != value.data() + value.size() ||
            result <= 0)
        {
            return fallback;
        }

        return result;
    }
};