#pragma once

#include "data/portal_dataset.hpp"
#include "data/portal_schema.hpp"
#include "data/portal_schema_mapper.hpp"
#include "core/graphql/server/portal_graphql_operation_registry.hpp"
#include "core/graphql/server/portal_graphql_operation_support.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class DepartmentMemberGraphQLOperations {
public:
    explicit DepartmentMemberGraphQLOperations(std::shared_ptr<PortalDataset> dataset)
        : dataset_(std::move(dataset))
    {
    }

    void registerWith(PortalGraphQLOperationRegistry& registry) {
        // Queries
        registry.registerQuery(
            "PortalDepartmentMembers",
            [this](const auto& variables) {
                return departmentMembersResponse(variables.json()["departmentId"].asInt());
            });
        registry.registerQuery(
            "PortalUserDepartments",
            [this](const auto& variables) {
                return userDepartmentsResponse(variables.json()["userId"].asInt());
            });
        registry.registerQuery(
            "PortalDepartmentMember",
            [this](const auto& variables) {
                return departmentMemberResponse(
                    variables.json()["departmentId"].asInt(),
                    variables.json()["userId"].asInt()
                );
            });
        // Mutations
        registry.registerMutation(
            "AddPortalDepartmentMember",
            [this](const auto& variables) {
                return addDepartmentMemberResponse(
                    variables.json()["departmentId"].asInt(),
                    variables.json()["userId"].asInt()
                );
            });
        registry.registerMutation(
            "RemovePortalDepartmentMember",
            [this](const auto& variables) {
                return removeDepartmentMemberResponse(
                    variables.json()["departmentId"].asInt(),
                    variables.json()["userId"].asInt()
                );
            });
    }

private:
    static Json::Value departmentMemberJson(
        const PortalDepartmentMember& value
    ) {
        return PortalSchemaMapper::toJson(PortalSchema::departmentMembers(), value);
    }

    int nextDepartmentMemberId() const {
        int id = 1;
        for (const auto& member: dataset_->departmentMembers()) {
            id = std::max(id,member.id + 1);
        }

        return id;
    }

    drogular::GraphQLResponse departmentMembersResponse(
        int id
    ) const {
        Json::Value members(Json::arrayValue);
        for (const auto& member: dataset_->departmentMembers()) {
            if (member.departmentId == id) {
                members.append(departmentMemberJson(member));
            }
        }

        Json::Value departmentMembers(Json::objectValue);
        departmentMembers["departmentMembers"] = members;

        return PortalGraphQLOperationSupport::response(departmentMembers);
    }

    drogular::GraphQLResponse userDepartmentsResponse(
        int id
    ) const {
        Json::Value members(Json::arrayValue);
        for (const auto& member: dataset_->departmentMembers()) {
            if (member.userId == id) {
                members.append(departmentMemberJson(member));
            }
        }

        Json::Value userDepartments(Json::objectValue);
        userDepartments["userDepartments"] = members;

        return PortalGraphQLOperationSupport::response(userDepartments);
    }

    drogular::GraphQLResponse departmentMemberResponse(
        int dep,
        int user
    ) const {
        Json::Value departments(Json::objectValue);
        departments["departmentMember"] = Json::Value();
        for (const auto& department: dataset_->departmentMembers()) {
            if (department.departmentId == dep && department.userId == user) {
                departments["departmentMember"] = departmentMemberJson(department);
                break;
            }
        }
        return PortalGraphQLOperationSupport::response(departments);
    }

    drogular::GraphQLResponse addDepartmentMemberResponse(
        int dep,
        int user
    ) {
        for (const auto& member: dataset_->departmentMembers()) {
            if (member.departmentId == dep && member.userId == user) {
                Json::Value department(Json::objectValue);
                department["addDepartmentMember"] = departmentMemberJson(member);

                return PortalGraphQLOperationSupport::response(department);
            }
        }

        PortalDepartmentMember v{
            nextDepartmentMemberId(),
            dep,user
        };
        dataset_->departmentMembers().push_back(v);

        Json::Value department(Json::objectValue);
        department["addDepartmentMember"] = departmentMemberJson(v);

        return PortalGraphQLOperationSupport::response(department);
    }

    drogular::GraphQLResponse removeDepartmentMemberResponse(
        int dep,
        int user
    ) {
        auto& members=
            dataset_->departmentMembers();
        auto size = members.size();

        std::erase_if(members,
            [=](const auto& value) {
                return value.departmentId == dep && value.userId == user;
            });

        Json::Value department(Json::objectValue);
        department["removeDepartmentMember"]= members.size() != size;

        return PortalGraphQLOperationSupport::response(department);
    }

    std::shared_ptr<PortalDataset> dataset_;
};