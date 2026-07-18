#pragma once

#include "data/models/portal_user.hpp"
#include "providers/user_provider.hpp"

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
        const PortalUserCreate& input
    ) override {
        PortalUser user;

        user.id = nextId_++;
        user.username = input.username;
        user.password = input.password;
        user.role = input.role;

        users_.push_back(user);

        return user;
    }

    PortalUser update(
        const PortalUserUpdate& input
    ) override {
        for (auto& user : users_) {
            if (user.id != input.id) {
                continue;
            }

            if (input.username.has_value()) {
                user.username =
                    *input.username;
            }

            if (input.role.has_value()) {
                user.role =
                    *input.role;
            }

            return user;
        }

        return {};
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

    std::optional<PortalUser> findByUsername(
        const std::string& username
    ) const override {
        for (const auto& user : all()) {
            if (user.username == username) {
                return user;
            }
        }

        return std::nullopt;
    }

private:
    std::vector<PortalUser> users_;
    int nextId_ = 1;
};