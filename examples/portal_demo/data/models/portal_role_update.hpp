#pragma once

#include <optional>
#include <string>

struct PortalRoleUpdate {
    int id = 0;

    std::optional<std::string> code;
    std::optional<std::string> title;
};