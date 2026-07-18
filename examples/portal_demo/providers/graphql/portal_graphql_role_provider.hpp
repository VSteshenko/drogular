#pragma once

#include "providers/role_provider.hpp"
#include "providers/graphql/documents/role_queries.hpp"
#include "providers/graphql/documents/role_mutations.hpp"
#include "providers/graphql/mappers/role_mapper.hpp"

#include <drogular/graphql_client.hpp>

#include <memory>

class PortalGraphQLRoleProvider final
    : public PortalRoleProvider
{
public:
    explicit PortalGraphQLRoleProvider(
        std::shared_ptr<drogular::GraphQLClient> client
    )
        : client_(std::move(client))
    {
    }

    std::vector<PortalRole> all() const override {
        const auto response =
            client_->execute(
                RoleQueries::all()
            );

        const auto roles =
            response.field("roles");

        if (!roles.has_value()) {
            return {};
        }

        return RoleMapper::fromList(*roles);
    }

    std::optional<PortalRole> findByCode(
        const std::string& code
    ) const override {
        const auto response =
            client_->execute(
                RoleQueries::findByCode(),
                RoleMapper::rolesVariables(code)
            );

        return RoleMapper::optionalRole(
            response.field("roleByCode")
        );
    }

    std::optional<PortalRole> findById(
        int id
    ) const override {
        const auto response =
            client_->execute(
                RoleQueries::findById(),
                RoleMapper::idVariables(id)
            );

        return RoleMapper::optionalRole(
            response.field("role")
        );
    }

    PortalRole create(
        const PortalRoleCreate& input
    ) override {
        const auto response =
            client_->execute(
                RoleMutations::create(input),
                RoleMapper::toVariables(input)
            );

        const auto value =
            response.field("createRole");

        if (!value.has_value()) {
            return {};
        }

        return RoleMapper::fromValue(*value);
    }

    PortalRole update(
        const PortalRoleUpdate& input
    ) override {
        const auto response =
            client_->execute(
                RoleMutations::update(input),
                RoleMapper::toVariables(input)
            );

        const auto value =
            response.field("updateRole");

        if (!value.has_value()) {
            return {};
        }

        return RoleMapper::fromValue(*value);
    }

    bool remove(
        int id
    ) override {
        const auto response =
            client_->execute(
                RoleMutations::remove(),
                RoleMapper::idVariables(id)
            );

        const auto value =
            response.field("removeRole");

        return value.has_value() &&
               value->asBool();
    }
private:
    std::shared_ptr<drogular::GraphQLClient> client_;
};