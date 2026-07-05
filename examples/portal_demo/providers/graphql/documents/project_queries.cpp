#include "project_queries.hpp"

#include <drogular/graphql.hpp>

drogular::gql::Query ProjectQueries::all() {
    return drogular::gql::query("PortalProjects")
        .select(
            drogular::gql::field("projects")
                .children({
                    drogular::gql::field("id"),
                    drogular::gql::field("title"),
                    drogular::gql::field("status"),
                    drogular::gql::field("ownerId"),
                    drogular::gql::field("projectTypeId")
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
                .children({
                    drogular::gql::field("id"),
                    drogular::gql::field("title"),
                    drogular::gql::field("status"),
                    drogular::gql::field("ownerId"),
                    drogular::gql::field("projectTypeId")
                })
        );
}