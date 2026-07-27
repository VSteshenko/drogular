#pragma once

#include "core/portal_string_utils.hpp"
#include "features/users/data/portal_user.hpp"
#include "features/users/providers/user_provider.hpp"

#include <drogular/pagination.hpp>

#include <algorithm>
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

    drogular::PagedResult<PortalUser> search(
        const PortalUserQuery& query
    ) const override {
        std::vector<PortalUser> result;

        const auto needle = query.search.has_value()
            ? portalAsciiLowercase(*query.search)
            : std::string();

        for (const auto& user : users_) {
            if (!needle.empty() &&
                portalAsciiLowercase(user.username).find(needle) ==
                    std::string::npos) {
                continue;
            }

            if (query.role.has_value() &&
                user.role != *query.role) {
                continue;
            }

            result.push_back(user);
        }

        auto sorting = query.sorting;
        if (sorting.empty()) {
            sorting.push_back({
                .field = "username",
                .direction = PortalSortDirection::Ascending
            });
        }

        const auto compare = []<typename T>(
            const T& left,
            const T& right,
            PortalSortDirection direction
        ) {
            return direction == PortalSortDirection::Ascending
                ? left < right
                : right < left;
        };

        std::stable_sort(
            result.begin(),
            result.end(),
            [&sorting, &compare](
                const PortalUser& left,
                const PortalUser& right
            ) {
                for (const auto& sort : sorting) {
                    if (sort.field == "username" &&
                        left.username != right.username) {
                        return compare(
                            left.username,
                            right.username,
                            sort.direction
                        );
                    }
                    if (sort.field == "role" &&
                        left.role != right.role) {
                        return compare(
                            left.role,
                            right.role,
                            sort.direction
                        );
                    }
                    if (sort.field == "id" &&
                        left.id != right.id) {
                        return compare(
                            left.id,
                            right.id,
                            sort.direction
                        );
                    }
                }
                return left.id < right.id;
            }
        );

        return drogular::paginate(
            result,
            query.page,
            query.pageSize
        );
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