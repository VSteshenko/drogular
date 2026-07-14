#pragma once

#include "../../../data/models/portal_project_filter.hpp"

#include <drogular/graphql.hpp>

class ProjectQueries {
public:
    static drogular::gql::Query all();

    static drogular::gql::Query search(
        const PortalProjectFilter& filter
    );

    static drogular::gql::Query findById(
        int id
    );
};