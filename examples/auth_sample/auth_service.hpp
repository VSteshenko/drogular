#pragma once

#include "auth_user.hpp"

#include <optional>
#include <string>

class AuthService {
public:
    /**
     * Authenticates a sample user.
     */
    std::optional<AuthUser> authenticate(
        const std::string& username,
        const std::string& password
    ) const;

    /**
     * Returns true when the user has the given role.
     */
    bool hasRole(
        const AuthUser& user,
        const std::string& role
    ) const;
};
