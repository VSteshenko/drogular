#pragma once

#include "features/users/data/portal_user.hpp"
#include "features/users/data/portal_user_update.hpp"

#include <drogular/graphql.hpp>

class UserMutations {
public:
    static drogular::gql::Mutation create(
        const PortalUser& user
    );

    static drogular::gql::Mutation update(
        const PortalUserUpdate& user
    );
};