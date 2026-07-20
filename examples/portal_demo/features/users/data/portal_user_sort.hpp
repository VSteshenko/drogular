#pragma once

#include "core/portal_sort_direction.hpp"

#include <string>

struct PortalUserSort {
    std::string field = "username";
    PortalSortDirection direction =
        PortalSortDirection::Ascending;
};