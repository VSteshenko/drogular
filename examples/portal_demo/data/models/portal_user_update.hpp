#pragma once

#include <optional>
#include <string>

struct PortalUserUpdate {
    int id = 0;

    std::optional<std::string> username;
    std::optional<std::string> role;
};