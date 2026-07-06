#pragma once

#include "../role_provider.hpp"
#include "documents/role_queries.hpp"
#include "mappers/role_mapper.hpp"

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
                RoleQueries::findByCode(code),
                RoleMapper::rolesVariables(code)
            );

        return RoleMapper::optionalRole(
            response.field("roleByCode")
        );
    }

private:
    std::shared_ptr<drogular::GraphQLClient> client_;
};