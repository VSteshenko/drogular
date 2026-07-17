#pragma once

#include "portal_project_sort.hpp"

#include <optional>
#include <string>
#include <vector>

struct PortalProjectFilter {
    std::optional<std::string> search;
    std::optional<std::string> status;
    std::optional<int> projectTypeId;
    std::optional<int> ownerId;

    std::vector<PortalProjectSort> sorting;
};