#include "role_queries.hpp"
#include "data/portal_graphql_document_builder.hpp"
#include "data/portal_schema.hpp"

drogular::gql::Query RoleQueries::all() {
    return PortalGraphQLDocumentBuilder::all(
        "PortalRoles",
        "roles",
        PortalSchema::roles()
    );
}

drogular::gql::Query RoleQueries::findByCode() {
    return PortalGraphQLDocumentBuilder::findByField(
        "PortalRoleByCode",
        "role",
        "code",
        "String!",
        PortalSchema::roles()
    );
}

drogular::gql::Query RoleQueries::findById() {
    return PortalGraphQLDocumentBuilder::findById(
        "PortalRoleById",
        "role",
        PortalSchema::roles()
    );
}