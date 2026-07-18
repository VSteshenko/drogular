#pragma once

#include "features/users/data/portal_user_query.hpp"

#include <drogular/graphql.hpp>

class UserQueries {
public:
    static drogular::gql::Query all();

    static drogular::gql::Query search(
        const PortalUserQuery& query
    );

    static drogular::gql::Query findByCredentials();
};