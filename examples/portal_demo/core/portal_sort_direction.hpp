#pragma once

#include <string_view>

enum class PortalSortDirection {
    Ascending,
    Descending
};

inline constexpr std::string_view toString(
    PortalSortDirection direction
) noexcept {
    return direction == PortalSortDirection::Descending
        ? "desc"
        : "asc";
}