#include "project_type_queries.hpp"
#include "../../../data/portal_graphql_selection_builder.hpp"
#include "../../../data/portal_schema.hpp"

drogular::gql::Query ProjectTypeQueries::all() {
    return drogular::gql::query("PortalProjectTypes")
        .select(
            drogular::gql::field("projectTypes")
            .children(
                PortalGraphQLSelectionBuilder::from(
                    PortalSchema::projectTypes()
                )
            )
        );
}

drogular::gql::Query ProjectTypeQueries::findById() {
    return drogular::gql::query("PortalProjectTypeById")
        .variable("id", "ID!")
        .select(
            drogular::gql::field("projectType")
                .arg("id", drogular::gql::variable("id"))
                .children(
                    PortalGraphQLSelectionBuilder::from(
                        PortalSchema::projectTypes()
                    )
                )
        );
}
