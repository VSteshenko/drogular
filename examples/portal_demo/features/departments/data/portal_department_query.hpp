#pragma once

#include "portal_department_sort.hpp"

#include <optional>
#include <string>
#include <vector>

struct PortalDepartmentQuery {
    std::optional<std::string> search;
    std::optional<bool> isActive;
    std::vector<PortalDepartmentSort> sorting;

    int page = 1;
    int pageSize = 10;
};