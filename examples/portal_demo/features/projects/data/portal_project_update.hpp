#pragma once

#include <string>
#include <optional>

struct PortalProjectUpdate {
    int id = 0;
    std::optional<std::string> title;
    std::optional<std::string> status;
    std::optional<int> projectTypeId;
};