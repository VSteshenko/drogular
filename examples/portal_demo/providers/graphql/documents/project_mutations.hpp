#pragma once

#include "../../../data/models/portal_project.hpp"
#include "../../../data/models/portal_project_update.hpp"

#include <drogular/graphql.hpp>

class ProjectMutations {
public:
    static drogular::gql::Mutation create(
        const PortalProject& project
    );

    static drogular::gql::Mutation update(
        const PortalProjectUpdate& project
    );

    static drogular::gql::Mutation remove(
        int id
    );
};