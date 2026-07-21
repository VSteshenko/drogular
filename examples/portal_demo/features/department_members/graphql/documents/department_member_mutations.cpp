#include "department_member_mutations.hpp"

#include <drogular/graphql.hpp>

namespace {
    std::vector<drogular::gql::Selection> selection() {
        return {
            drogular::gql::field("id"),
            drogular::gql::field("departmentId"),
            drogular::gql::field("userId")
        };
    }
}

drogular::gql::Mutation DepartmentMemberMutations::add(int, int) {
    return drogular::gql::mutation("AddPortalDepartmentMember")
        .variable("departmentId", "ID!")
        .variable("userId", "ID!")
        .select(
            drogular::gql::field("addDepartmentMember")
                .arg(
                    "departmentId",
                    drogular::gql::variable("departmentId")
                )
                .arg(
                    "userId",
                    drogular::gql::variable("userId")
                )
                .children(selection())
        );
}

drogular::gql::Mutation DepartmentMemberMutations::remove(int, int) {
    return drogular::gql::mutation("RemovePortalDepartmentMember")
        .variable("departmentId", "ID!")
        .variable("userId", "ID!")
        .select(
            drogular::gql::field("removeDepartmentMember")
                .arg(
                    "departmentId",
                    drogular::gql::variable("departmentId")
                )
                .arg(
                    "userId",
                    drogular::gql::variable("userId")
                )
        );
}