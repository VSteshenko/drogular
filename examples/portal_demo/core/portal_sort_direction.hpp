#pragma once

#include <string_view>

enum class PortalSortDirection {
    Ascending,
    Descending
};

inline constexpr const char* toString(
    PortalSortDirection direction
) noexcept {
    return direction == PortalSortDirection::Descending
        ? "desc"
        : "asc";
}