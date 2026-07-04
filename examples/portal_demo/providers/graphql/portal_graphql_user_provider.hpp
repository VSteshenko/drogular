#pragma once

#include "../user_provider.hpp"
#include "documents/user_queries.hpp"
#include "documents/user_mutations.hpp"
#include "mappers/user_mapper.hpp"

#include <drogular/graphql_client.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

class PortalGraphQLUserProvider final
    : public PortalUserProvider
{
public:
    explicit PortalGraphQLUserProvider(
        std::shared_ptr<drogular::GraphQLClient> client
    )
        : client_(std::move(client))
    {
    }

    std::vector<PortalUser> all() const override {
        const auto response =
            client_->execute(
                UserQueries::all()
            );

        const auto users =
            response.field("users");

        if (!users.has_value()) {
            return {};
        }

        return UserMapper::fromList(*users);
    }

    std::optional<PortalUser> findByCredentials(
        const std::string& username,
        const std::string& password
    ) const override {
        const auto response =
            client_->execute(
                UserQueries::findByCredentials(),
                UserMapper::credentialsVariables(
                    username,
                    password
                )
            );

        return UserMapper::optionalUser(
            response.field("userByCredentials")
        );
    }

    bool exists(
        const std::string& username
    ) const override {
        const auto users =
            all();

        for (const auto& user : users) {
            if (user.username == username) {
                return true;
            }
        }

        return false;
    }

    PortalUser create(
        PortalUser user
    ) override {
        const auto response =
            client_->execute(
                UserMutations::create(user),
                UserMapper::toVariables(user)
            );

        const auto created =
            response.field("createUser");

        if (!created.has_value()) {
            return user;
        }

        return UserMapper::fromValue(*created);
    }

    bool update(
        PortalUser user
    ) override {
        const auto response =
            client_->execute(
                UserMutations::update(user),
                UserMapper::toVariables(user)
            );

        return response.field("updateUser").has_value() &&
               !response.field("updateUser")->isNull();
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
    std::shared_ptr<drogular::GraphQLClient> client_;
};