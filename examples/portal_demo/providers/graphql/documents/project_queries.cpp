#include "project_queries.hpp"
#include "../../../data/portal_graphql_selection_builder.hpp"
#include "../../../data/portal_schema.hpp"

#include <drogular/graphql.hpp>

drogular::gql::Query ProjectQueries::all() {
    return drogular::gql::query("PortalProjects")
        .select(
            drogular::gql::field("projects")
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