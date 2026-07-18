#include "role_mutations.hpp"
#include "data/portal_graphql_document_builder.hpp"
#include "data/portal_schema.hpp"

drogular::gql::Mutation RoleMutations::create(
    const PortalRoleCreate&
) {
    return PortalGraphQLDocumentBuilder::mutation(
        "CreatePortalRole",
        "createRole",
        "role",
        "RoleCreateInput!",
        PortalSchema::roles()
    );
}

drogular::gql::Mutation RoleMutations::update(
    const PortalRoleUpdate&
) {
    return PortalGraphQLDocumentBuilder::mutation(
        "UpdatePortalRole",
        "updateRole",
        "role",
        "RoleUpdateInput!",
        PortalSchema::roles()
    );
}

drogular::gql::Mutation RoleMutations::remove() {
    return drogular::gql::mutation(
        "RemovePortalRole"
    )
        .variable(
            "id",
            "ID!"
        )
        .select(
            drogular::gql::field("removeRole")
                .arg(
                    "id",
                    drogular::gql::variable("id")
                )
        );
}