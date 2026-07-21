#pragma once

#include <drogular/graphql.hpp>

class DepartmentMemberMutations {
public:
    static drogular::gql::Mutation add(
        int departmentId,
        int userId
    );

    static drogular::gql::Mutation remove(
        int departmentId,
        int userId
    );
};