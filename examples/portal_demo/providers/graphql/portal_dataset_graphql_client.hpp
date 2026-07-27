#pragma once

#include "data/portal_dataset.hpp"
#include "features/projects/graphql/server/project_graphql_operations.hpp"
#include "features/users/graphql/server/user_graphql_operations.hpp"
#include "features/departments/graphql/server/department_graphql_operations.hpp"
#include "features/department_members/graphql/server/department_member_graphql_operations.hpp"
#include "features/roles/graphql/server/role_graphql_operations.hpp"
#include "features/project_types/graphql/server/project_type_graphql_operations.hpp"
#include "core/graphql/server/portal_graphql_operation_registry.hpp"

#include <drogular/static_graphql_client.hpp>

#include <memory>
#include <string>

class PortalDatasetGraphQLClient final : public drogular::GraphQLClient {
public:
    explicit PortalDatasetGraphQLClient(std::shared_ptr<PortalDataset> dataset)
        : projects_(dataset),
          users_(dataset),
          departments_(dataset),
          departmentMembers_(dataset),
          roles_(dataset),
          projectTypes_(std::move(dataset))
    {
        projects_.registerWith(registry_);
        users_.registerWith(registry_);
        departments_.registerWith(registry_);
        departmentMembers_.registerWith(registry_);
        roles_.registerWith(registry_);
        projectTypes_.registerWith(registry_);
    }

    drogular::GraphQLResponse execute(
        const drogular::gql::Query& query,
        const drogular::GraphQLVariables& variables = {}
    ) override {
        return registry_.executeQuery(
            operationName(query.toString(), "query"),
            variables
        );
    }

    drogular::GraphQLResponse execute(
        const drogular::gql::Mutation& mutation,
        const drogular::GraphQLVariables& variables = {}
    ) override {
        return registry_.executeMutation(
            operationName(mutation.toString(), "mutation"),
            variables
        );
    }

    drogular::GraphQLResponse executeRequest(
        const drogular::GraphQLRequest&
    ) override {
        return registry_.executeQuery("", {});
    }

    static std::string operationName(
        const std::string& text,
        const std::string& type
    ) {
        const auto prefix = type + " ";
        if (!text.starts_with(prefix)) {
            return "";
        }

        const auto start = prefix.size();
        const auto end = text.find_first_of(" (", start);

        return end == std::string::npos
            ? text.substr(start)
            : text.substr(start, end - start);
    }

private:
    PortalGraphQLOperationRegistry registry_;
    ProjectGraphQLOperations projects_;
    UserGraphQLOperations users_;
    DepartmentGraphQLOperations departments_;
    DepartmentMemberGraphQLOperations departmentMembers_;
    RoleGraphQLOperations roles_;
    ProjectTypeGraphQLOperations projectTypes_;
};