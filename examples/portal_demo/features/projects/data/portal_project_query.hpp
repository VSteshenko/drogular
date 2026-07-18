#pragma once

#include "features/projects/data/portal_project_sort.hpp"

#include <optional>
#include <string>
#include <vector>

struct PortalProjectQuery {
    std::optional<std::string> search;
    std::optional<std::string> status;
    std::optional<int> projectTypeId;
    std::optional<int> ownerId;

    std::vector<PortalProjectSort> sorting;

    int page = 1;
    int pageSize = 10;
};