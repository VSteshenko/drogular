#pragma once

#include "features/project_types/data/portal_project_type_create.hpp"
#include "features/project_types/data/portal_project_type_update.hpp"

#include <drogular/graphql.hpp>

class ProjectTypeMutations {
public:
    static drogular::gql::Mutation create(
        const PortalProjectTypeCreate& input
    );

    static drogular::gql::Mutation update(
        const PortalProjectTypeUpdate& input
    );

    static drogular::gql::Mutation remove();
};