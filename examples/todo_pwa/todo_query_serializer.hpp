#pragma once

#include "todo_query.hpp"

#include <drogular/url.hpp>

#include <string>

class TodoQuerySerializer {
public:
    static std::string toQueryString(const TodoQuery& query) {
        std::string result;

        addIf(result,
            !query.search.empty(),
            "search",
            query.search
        );
        addIf(result,
            query.pageSize != 5,
            "pageSize",
            std::to_string(query.pageSize)
        );
        addIf(result,
            query.page > 1,
            "page",
            std::to_string(query.page)
        );

        return result;
    }

private:
    static void addIf(
        std::string& result,
        bool condition,
        const std::string& name,
        const std::string& value
    ) {
        if (!condition) {
            return;
        }

        result += result.empty() ? '?' : '&';
        result += name;
        result += '=';
        result += drogular::Url::encode(value);
    }
};