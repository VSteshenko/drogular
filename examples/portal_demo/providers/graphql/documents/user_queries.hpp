#pragma once

#include <drogular/graphql.hpp>

class UserQueries {
public:
    static drogular::gql::Query all();

    static drogular::gql::Query findByCredentials();
};