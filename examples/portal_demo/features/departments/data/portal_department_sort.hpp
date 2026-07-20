#pragma once

#include "core/portal_sort_direction.hpp"

#include <string>

struct PortalDepartmentSort {
    std::string field = "name";
    PortalSortDirection direction =
        PortalSortDirection::Ascending;
};