#pragma once

#include "features/departments/data/portal_department_query.hpp"

#include <drogular/graphql.hpp>

class DepartmentQueries {
public:
    static drogular::gql::Query all();

    static drogular::gql::Query search(
        const PortalDepartmentQuery& query
    );

    static drogular::gql::Query findById(
        int id
    );
};