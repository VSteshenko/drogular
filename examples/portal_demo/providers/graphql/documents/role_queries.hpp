#pragma once

#include <drogular/graphql.hpp>

#include <string>

class RoleQueries {
public:
    static drogular::gql::Query all();

    static drogular::gql::Query findById(
        int id
    );

    static drogular::gql::Query findByCode(
        const std::string& code
    );
};