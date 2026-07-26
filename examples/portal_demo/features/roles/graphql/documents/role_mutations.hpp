#pragma once

#include "features/roles/data/portal_role_create.hpp"
#include "features/roles/data/portal_role_update.hpp"

#include <drogular/graphql.hpp>

class RoleMutations {
public:
    static drogular::gql::Mutation create(
        const PortalRoleCreate& input
    );

    static drogular::gql::Mutation update(
        const PortalRoleUpdate& input
    );

    static drogular::gql::Mutation remove();
};