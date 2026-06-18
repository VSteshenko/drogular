#include "auth_service.hpp"

std::optional<AuthUser> AuthService::authenticate(
    const std::string& username,
    const std::string& password
) const {
    if (username == "admin" &&
        password == "password") {
        return AuthUser{
            .id = 1,
            .username = "admin",
            .role = "admin"
        };
    }

    if (username == "user" &&
        password == "password") {
        return AuthUser{
            .id = 2,
            .username = "user",
            .role = "user"
        };
    }

    return std::nullopt;
}

bool AuthService::hasRole(
    const AuthUser& user,
    const std::string& role
) const {
    return user.role == role;
}
