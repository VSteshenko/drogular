#pragma once

#include <string>

struct TodoQuery {
    std::string search;
    int page{1};
    int pageSize{5};
};