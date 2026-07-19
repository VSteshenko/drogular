#pragma once

#include <optional>
#include <string>

struct PortalDepartmentUpdate {
    int id = 0;
    std::optional<std::string> name;
    std::optional<std::string> description;
    std::optional<int> managerId;
    std::optional<bool> isActive;
};