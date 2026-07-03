#pragma once

#include "../../../portal_project.hpp"

#include <drogular/graphql.hpp>

class ProjectMutations {
public:
    static drogular::gql::Mutation create(
        const PortalProject& project
    );

    static drogular::gql::Mutation update(
        const PortalProject& project
    );

    static drogular::gql::Mutation remove(
        int id
    );
};