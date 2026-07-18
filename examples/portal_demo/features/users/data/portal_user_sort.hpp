#pragma once

#include <string>

enum class PortalUserSortDirection {
    Ascending,
    Descending
};

struct PortalUserSort {
    std::string field;

    PortalUserSortDirection direction =
        PortalUserSortDirection::Ascending;
};

inline std::string toString(
    PortalUserSortDirection direction
) {
    return direction ==
           PortalUserSortDirection::Descending
        ? "desc"
        : "asc";
}