#pragma once

#include <drogular/graphql.hpp>

class DepartmentMemberQueries {
public:
    static drogular::gql::Query byDepartment(
        int departmentId
    );

    static drogular::gql::Query byUser(
        int userId
    );

    static drogular::gql::Query find(
        int departmentId,
        int userId
    );
};