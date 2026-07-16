#pragma once

#include <optional>
#include <string>

struct PortalProjectFilter {
    std::optional<std::string> search;
    std::optional<std::string> status;
    std::optional<int> projectTypeId;
    std::optional<int> ownerId;
};