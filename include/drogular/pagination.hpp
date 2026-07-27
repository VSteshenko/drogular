#pragma once

#include <drogular/paged_result.hpp>

#include <algorithm>
#include <cstddef>
#include <vector>

namespace drogular {

template<typename T>
PagedResult<T> paginate(
    const std::vector<T>& values,
    int requestedPage,
    int requestedPageSize
) {
    PagedResult<T> result;
    result.pageSize = std::max(1, requestedPageSize);
    result.totalItems = static_cast<int>(values.size());
    result.totalPages = std::max(
        1,
        (result.totalItems + result.pageSize - 1) /
            result.pageSize
    );
    result.page = std::clamp(
        requestedPage,
        1,
        result.totalPages
    );

    const auto beginIndex =
        static_cast<std::size_t>(result.page - 1) *
        static_cast<std::size_t>(result.pageSize);
    const auto endIndex = std::min(
        values.size(),
        beginIndex + static_cast<std::size_t>(result.pageSize)
    );

    result.items.assign(
        values.begin() + static_cast<std::ptrdiff_t>(beginIndex),
        values.begin() + static_cast<std::ptrdiff_t>(endIndex)
    );

    return result;
}

} // namespace drogular