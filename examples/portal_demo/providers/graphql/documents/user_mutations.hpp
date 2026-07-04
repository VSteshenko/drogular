#pragma once

#include "../../../portal_user.hpp"

#include <drogular/graphql.hpp>

class UserMutations {
public:
    static drogular::gql::Mutation create(
        const PortalUser& user
    );

    static drogular::gql::Mutation update(
        const PortalUser& user
    );
};