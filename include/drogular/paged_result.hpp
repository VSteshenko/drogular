#pragma once

#include <vector>

namespace drogular {

template<typename T>
struct PagedResult {
    std::vector<T> items;

    int page{1};
    int pageSize{10};
    int totalItems{0};
    int totalPages{1};

    bool hasPrevious() const noexcept {
        return page > 1;
    }

    bool hasNext() const noexcept {
        return page < totalPages;
    }
};

} // namespace drogular