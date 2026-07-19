#pragma once

#include <string>

struct PortalDepartmentCreate {
    std::string name;
    std::string description;

    int managerId = 0;
    bool isActive = true;
};