#pragma once

#include <drogular/graphql.hpp>

class RoleQueries {
public:
    static drogular::gql::Query all();

    static drogular::gql::Query findById();

    static drogular::gql::Query findByCode();
};