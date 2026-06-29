#pragma once

#include "portal_user.hpp"
#include "portal_user_provider.hpp"

#include <optional>
#include <string>
#include <vector>

class PortalMemoryUserProvider final
    : public PortalUserProvider
{
public:
    PortalUser create(
        PortalUser user
    ) override {
        users_.push_back(user);
        return user;
    }

    std::optional<PortalUser> findByCredentials(
        const std::string& username,
        const std::string& password
    ) const override {
        for (const auto& user : users_) {
            if (user.username == username &&
                user.password == password) {
                return user;
            }
        }

        return std::nullopt;
    }

    std::vector<PortalUser> all() const override {
        return users_;
    }

    bool exists(
        const std::string& username
    ) const override {
        for (const auto& user : users_) {
            if (user.username == username) {
                return true;
            }
        }

        return false;
    }

private:
    std::vector<PortalUser> users_{
        {"admin", "admin", "admin"},
        {"user", "user", "user"}
    };
};