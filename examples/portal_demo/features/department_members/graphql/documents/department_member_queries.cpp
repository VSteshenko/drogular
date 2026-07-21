#include "department_member_queries.hpp"

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

drogular::gql::Query DepartmentMemberQueries::byDepartment(int) {
    return drogular::gql::query("PortalDepartmentMembers")
        .variable("departmentId", "ID!")
        .select(
            drogular::gql::field("departmentMembers")
                .arg(
                    "departmentId",
                    drogular::gql::variable("departmentId")
                )
                .children(selection())
        );
}

drogular::gql::Query DepartmentMemberQueries::byUser(int) {
    return drogular::gql::query("PortalUserDepartments")
        .variable("userId", "ID!")
        .select(
            drogular::gql::field("userDepartments")
                .arg(
                    "userId",
                    drogular::gql::variable("userId")
                )
                .children(selection())
        );
}

drogular::gql::Query DepartmentMemberQueries::find(int, int) {
    return drogular::gql::query("PortalDepartmentMember")
        .variable("departmentId", "ID!")
        .variable("userId", "ID!")
        .select(
            drogular::gql::field("departmentMember")
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