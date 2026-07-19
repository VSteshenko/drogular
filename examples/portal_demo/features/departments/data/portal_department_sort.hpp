#pragma once

#include <string>

enum class PortalDepartmentSortDirection {
    Ascending,
    Descending
};

inline std::string toString(PortalDepartmentSortDirection value) {
    return value ==
        PortalDepartmentSortDirection::Descending
        ? "desc"
        : "asc";
}

struct PortalDepartmentSort {
    std::string field = "name";

    PortalDepartmentSortDirection direction =
        PortalDepartmentSortDirection::Ascending;
};