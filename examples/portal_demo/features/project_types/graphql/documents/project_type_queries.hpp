#pragma once

#include <drogular/graphql.hpp>

class ProjectTypeQueries {
public:
    static drogular::gql::Query all();

    static drogular::gql::Query findById();
};