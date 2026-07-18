#pragma once

#include "features/projects/data/portal_project_query.hpp"

#include <drogular/graphql.hpp>

class ProjectQueries {
public:
    static drogular::gql::Query all();

    static drogular::gql::Query search(
        const PortalProjectQuery& query
    );

    static drogular::gql::Query findById(
        int id
    );
};