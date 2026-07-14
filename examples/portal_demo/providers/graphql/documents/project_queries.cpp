#include "project_queries.hpp"
#include "../../../data/portal_graphql_document_builder.hpp"
#include "../../../data/portal_schema.hpp"

#include <drogular/graphql.hpp>

drogular::gql::Query ProjectQueries::all() {
    return PortalGraphQLDocumentBuilder::all(
        "PortalProjects",
        "projects",
        PortalSchema::projects()
    );
}

drogular::gql::Query ProjectQueries::search(
    const PortalProjectFilter&
) {
    return drogular::gql::query(
        "SearchPortalProjects"
    )
        .variable(
            "search",
            "String"
        )
        .variable(
            "status",
            "String"
        )
        .select(
            drogular::gql::field("projects")
                .arg(
                    "search",
                    drogular::gql::variable("search")
                )
                .arg(
                    "status",
                    drogular::gql::variable("status")
                )
                .children(
                    PortalGraphQLSelectionBuilder::from(
                        PortalSchema::projects()
                    )
                )
        );
}

drogular::gql::Query ProjectQueries::findById(
    int id
) {
    return drogular::gql::query("PortalProjectById")
        .variable("id", "ID!")
        .select(
            drogular::gql::field("project")
                .arg(
                    "id",
                    drogular::gql::variable("id")
                )
                .children(
                    PortalGraphQLSelectionBuilder::from(
                        PortalSchema::projects()
                    )
                )
        );
}