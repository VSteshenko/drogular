#pragma once

#include "core/graphql/server/portal_graphql_server.hpp"
#include "features/department_members/graphql/server/department_member_graphql_operations.hpp"
#include "features/departments/graphql/server/department_graphql_operations.hpp"
#include "features/project_types/graphql/server/project_type_graphql_operations.hpp"
#include "features/projects/graphql/server/project_graphql_operations.hpp"
#include "features/roles/graphql/server/role_graphql_operations.hpp"
#include "features/users/graphql/server/user_graphql_operations.hpp"

#include <memory>

inline std::shared_ptr<PortalGraphQLServer> createPortalGraphQLServer(
    const std::shared_ptr<PortalDataset>& dataset
) {
    auto server =
        std::make_shared<PortalGraphQLServer>();

    server
        ->add<ProjectGraphQLOperations>(dataset)
        .add<UserGraphQLOperations>(dataset)
        .add<DepartmentGraphQLOperations>(dataset)
        .add<DepartmentMemberGraphQLOperations>(dataset)
        .add<RoleGraphQLOperations>(dataset)
        .add<ProjectTypeGraphQLOperations>(dataset);

    return server;
}