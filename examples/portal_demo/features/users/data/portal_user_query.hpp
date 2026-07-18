#pragma once

#include "features/users/data/portal_user_sort.hpp"

#include <optional>
#include <string>
#include <vector>

struct PortalUserQuery {
    std::optional<std::string> search;
    std::optional<std::string> role;

    std::vector<PortalUserSort> sorting;

    int page = 1;
    int pageSize = 10;
};