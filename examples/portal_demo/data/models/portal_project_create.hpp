#pragma once

#include <string>
#include <optional>

struct PortalProjectCreate {
    std::string title;
    std::optional<std::string> status;
    int projectTypeId = 0;
};