#pragma once

#include "portal_user.hpp"

#include <optional>
#include <string>
#include <vector>

class PortalUserRepository {
public:
    std::optional<PortalUser> findByCredentials(
        const std::string& username,
        const std::string& password
    ) const {
        for (const auto& user : users_) {
            if (user.username == username &&
                user.password == password) {
                return user;
            }
        }

        return std::nullopt;
    }

private:
    std::vector<PortalUser> users_{
        {"admin", "admin", "admin"},
        {"user", "user", "user"}
    };
};