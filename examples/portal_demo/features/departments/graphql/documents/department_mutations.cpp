#include "department_mutations.hpp"
#include "data/portal_graphql_document_builder.hpp"
#include "data/portal_schema.hpp"

drogular::gql::Mutation DepartmentMutations::create(
    const PortalDepartment&
) {
    return PortalGraphQLDocumentBuilder::mutation(
        "CreatePortalDepartment",
        "createDepartment",
        "department",
        "DepartmentInput!",
        PortalSchema::departments()
    );
}

drogular::gql::Mutation DepartmentMutations::update(
    const PortalDepartmentUpdate&
) {
    return PortalGraphQLDocumentBuilder::mutation(
        "UpdatePortalDepartment",
        "updateDepartment",
        "department",
        "DepartmentUpdateInput!",
        PortalSchema::departments()
    );
}