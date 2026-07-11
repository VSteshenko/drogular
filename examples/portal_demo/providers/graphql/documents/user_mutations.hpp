#pragma once

#include "../../../data/models/portal_user.hpp"
#include "../../../data/models/portal_user_update.hpp"

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