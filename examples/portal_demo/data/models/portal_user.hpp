#pragma once

#include <string>

struct PortalUser {
    int id = 0;
    std::string username;
    std::string password;
    std::string role;
};