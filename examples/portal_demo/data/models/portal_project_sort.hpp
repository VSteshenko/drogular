#pragma once

#include <string>

enum class PortalProjectSortDirection {
    Ascending,
    Descending
};

struct PortalProjectSort {
    std::string field;

    PortalProjectSortDirection direction =
        PortalProjectSortDirection::Ascending;
};

inline std::string toString(
    PortalProjectSortDirection direction
) {
    return direction ==
           PortalProjectSortDirection::Descending
        ? "desc"
        : "asc";
}