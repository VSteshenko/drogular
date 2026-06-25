#pragma once

#include "portal_user.hpp"

#include <optional>
#include <string>
#include <vector>
#include <utility>

class PortalUserRepository {
public:
    void create(
        std::string username,
        std::string role
    ) {
        users_.push_back({
            .username = std::move(username),
            .password = username,
            .role = std::move(role)
        });
    }

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

    std::vector<PortalUser> all() {
        return users_;
    }

private:
    std::vector<PortalUser> users_{
        {"admin", "admin", "admin"},
        {"user", "user", "user"}
    };
};