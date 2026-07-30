#pragma once

#include "todo_query.hpp"

#include <drogular/query_string_builder.hpp>

#include <string>

class TodoQuerySerializer {
public:
    static std::string toQueryString(const TodoQuery& query) {
        return drogular::QueryStringBuilder{}
            .addNonEmpty("search", query.search)
            .addIf(query.pageSize != 5, "pageSize", query.pageSize)
            .addIf(query.page > 1, "page", query.page)
            .build();
    }
};