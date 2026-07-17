#pragma once

#include <vector>

template<typename T>
struct PortalPage {
    std::vector<T> items;

    int page = 1;
    int pageSize = 10;
    int totalItems = 0;
    int totalPages = 1;
};