#pragma once

#include "features/departments/data/portal_department.hpp"
#include "features/departments/data/portal_department_update.hpp"

#include <drogular/graphql.hpp>

class DepartmentMutations {
public:
    static drogular::gql::Mutation create(
        const PortalDepartment& department
    );

    static drogular::gql::Mutation update(
        const PortalDepartmentUpdate& input
    );
};