#pragma once

#include <drogular/graphql.hpp>

class ProjectQueries {
public:
    static drogular::gql::Query all();

    static drogular::gql::Query findById(
        int id
    );
};