#pragma once

#include "todo_query.hpp"

#include <drogular/request_parameters.hpp>

class TodoQueryParser {
public:
    static TodoQuery fromRequest(const drogon::HttpRequestPtr& request) {
        const drogular::RequestParameters parameters(request);

        TodoQuery query;
        query.search = parameters.value("search");
        query.page = parameters.positiveIntegerOr("page", 1);
        query.pageSize = parameters.boundedPositiveIntegerOr(
            "pageSize",
            5,
            20
        );

        return query;
    }
};