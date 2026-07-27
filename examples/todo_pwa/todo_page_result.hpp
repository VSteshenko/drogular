#pragma once

#include "todo.hpp"

#include <vector>

struct TodoPageResult {
    std::vector<Todo> items;

    int page{1};
    int pageSize{5};
    int totalItems{0};
    int totalPages{1};
};