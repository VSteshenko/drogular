#pragma once

#include "features/department_members/providers/department_member_provider.hpp"
#include "features/department_members/graphql/documents/department_member_queries.hpp"
#include "features/department_members/graphql/documents/department_member_mutations.hpp"
#include "features/department_members/graphql/mappers/department_member_mapper.hpp"

#include <drogular/graphql_client.hpp>

#include <memory>

class PortalGraphQLDepartmentMemberProvider final
    : public PortalDepartmentMemberProvider
{
public:
    explicit PortalGraphQLDepartmentMemberProvider(
        std::shared_ptr<drogular::GraphQLClient> client
    )
        : client_(std::move(client))
    {
    }

    std::vector<PortalDepartmentMember> membersOfDepartment(
        int id
    ) const override {
        auto response =
            client_->execute(
                DepartmentMemberQueries::byDepartment(id),
                DepartmentMemberMapper::departmentVariables(id)
            );
        auto value=
            response.field("departmentMembers");

        return value
            ? DepartmentMemberMapper::fromList(*value)
            : std::vector<PortalDepartmentMember>{};
    }

    std::vector<PortalDepartmentMember> departmentsOfUser(
        int id
    ) const override {
        auto response =
            client_->execute(
                DepartmentMemberQueries::byUser(id),
                DepartmentMemberMapper::userVariables(id)
            );
        auto value=
            response.field("userDepartments");

        return value
            ? DepartmentMemberMapper::fromList(*value)
            : std::vector<PortalDepartmentMember>{};
    }

    std::optional<PortalDepartmentMember> find(
        int departmentId,
        int userId
    ) const override {
        auto response =
            client_->execute(
                DepartmentMemberQueries::find(departmentId, userId),
                DepartmentMemberMapper::variables(departmentId, userId)
            );

        return DepartmentMemberMapper::optionalMember(
            response.field("departmentMember")
        );
    }

    PortalDepartmentMember addMember(
        int departmentId,
        int userId
    ) override {
        auto response =
            client_->execute(
                DepartmentMemberMutations::add(departmentId, userId),
                DepartmentMemberMapper::variables(departmentId, userId)
            );

        return DepartmentMemberMapper::fromValue(response.data()["addDepartmentMember"]);
    }

    bool removeMember(
        int departmentId,
        int userId
    ) override {
        auto response = client_->execute(
            DepartmentMemberMutations::remove(departmentId, userId),
            DepartmentMemberMapper::variables(departmentId, userId));

        auto value= response.field("removeDepartmentMember");

        return value && value->asBool();
    }

private:
    std::shared_ptr<drogular::GraphQLClient> client_;
};