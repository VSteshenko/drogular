#pragma once

#include <string>

struct PortalDepartment {
    int id = 0;
    std::string name;
    std::string description;
    int managerId = 0;
    bool isActive = true;
};