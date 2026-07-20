#pragma once

#include "data/models/portal_page.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

template<typename T>
PortalPage<T> paginate(
    const std::vector<T>& values,
    int requestedPage,
    int requestedPageSize
) {
    PortalPage<T> page;
    page.page = std::max(1, requestedPage);
    page.pageSize = std::max(1, requestedPageSize);
    page.totalItems = static_cast<int>(values.size());
    page.totalPages = std::max(
        1,
        (page.totalItems + page.pageSize - 1) /
            page.pageSize
    );

    const auto beginIndex =
        static_cast<std::size_t>(page.page - 1) *
        static_cast<std::size_t>(page.pageSize);

    if (beginIndex >= values.size()) {
        return page;
    }

    const auto endIndex = std::min(
        values.size(),
        beginIndex + static_cast<std::size_t>(page.pageSize)
    );

    page.items.assign(
        values.begin() + static_cast<std::ptrdiff_t>(beginIndex),
        values.begin() + static_cast<std::ptrdiff_t>(endIndex)
    );

    return page;
}