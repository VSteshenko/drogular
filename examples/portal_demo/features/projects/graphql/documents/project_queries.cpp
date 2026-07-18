#include "project_queries.hpp"
#include "data/portal_graphql_document_builder.hpp"
#include "data/portal_schema.hpp"

#include <drogular/graphql.hpp>

drogular::gql::Query ProjectQueries::all() {
    return PortalGraphQLDocumentBuilder::all(
        "PortalProjects",
        "projects",
        PortalSchema::projects()
    );
}

drogular::gql::Query ProjectQueries::search(
    const PortalProjectQuery&
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
        .variable(
            "projectTypeId",
            "ID"
        )
        .variable(
            "ownerId",
            "ID"
        )
        .variable(
            "sorting",
            "[ProjectSortInput!]"
        )
        .variable(
            "page",
            "Int!"
        )
        .variable(
            "pageSize",
            "Int!"
        )
        .select(
            drogular::gql::field("projectPage")
                .arg(
                    "search",
                    drogular::gql::variable("search")
                )
                .arg(
                    "status",
                    drogular::gql::variable("status")
                )
                .arg(
                    "projectTypeId",
                    drogular::gql::variable(
                        "projectTypeId"
                    )
                )
                .arg(
                    "ownerId",
                    drogular::gql::variable("ownerId")
                )
                .arg(
                    "sorting",
                    drogular::gql::variable("sorting")
                )
                .arg(
                    "page",
                    drogular::gql::variable("page")
                )
                .arg(
                    "pageSize",
                    drogular::gql::variable("pageSize")
                )
                .children({
                    drogular::gql::field("items")
                        .children(
                            PortalGraphQLSelectionBuilder::from(
                                PortalSchema::projects()
                            )
                        ),
                    drogular::gql::field("page"),
                    drogular::gql::field("pageSize"),
                    drogular::gql::field("totalItems"),
                    drogular::gql::field("totalPages")
                })
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