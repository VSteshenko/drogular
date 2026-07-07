#include "project_type_queries.hpp"
#include "../../../data/portal_graphql_document_builder.hpp"
#include "../../../data/portal_schema.hpp"

drogular::gql::Query ProjectTypeQueries::all() {
    return PortalGraphQLDocumentBuilder::all(
        "PortalProjectTypes",
        "projectTypes",
        PortalSchema::projectTypes()
    );
}

drogular::gql::Query ProjectTypeQueries::findById() {
    return PortalGraphQLDocumentBuilder::findById(
        "PortalProjectTypeById",
        "projectType",
        PortalSchema::projectTypes()
    );
}