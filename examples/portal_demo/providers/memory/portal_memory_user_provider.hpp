#pragma once

#include "../../portal_user.hpp"
#include "../user_provider.hpp"

#include <optional>
#include <string>
#include <vector>

class PortalMemoryUserProvider final
    : public PortalUserProvider
{
public:
    explicit PortalMemoryUserProvider(
        std::vector<PortalUser> users
    )
        : users_(std::move(users))
    {
    }

    PortalMemoryUserProvider() {
        users_.push_back({
            .username = "admin",
            .password = "admin",
            .role = "admin"
        });
        users_.push_back({
            .username = "user",
            .password = "user",
            .role = "user"
        });
    }

    PortalUser create(
        PortalUser user
    ) override {
        users_.push_back(user);
        return user;
    }

    bool update(
        PortalUser user
    ) override {
        for (auto& existing : users_) {
            if (existing.id == user.id) {
                existing = std::move(user);
                return true;
            }
        }

        return false;
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

    std::optional<PortalUser> findById(int id) const override {
        for (const auto& user : all()) {
            if (user.id == id) {
                return user;
            }
        }

        return std::nullopt;
    }

private:
    std::vector<PortalUser> users_;
};