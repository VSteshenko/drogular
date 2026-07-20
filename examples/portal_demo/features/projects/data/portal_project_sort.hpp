#pragma once

#include "core/portal_sort_direction.hpp"

#include <string>

struct PortalProjectSort {
    std::string field = "title";
    PortalSortDirection direction =
        PortalSortDirection::Ascending;
};