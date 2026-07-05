#include "project_type_queries.hpp"

drogular::gql::Query ProjectTypeQueries::all() {
    return drogular::gql::query("PortalProjectTypes")
        .select(
            drogular::gql::field("projectTypes")
                .children({
                    drogular::gql::field("id"),
                    drogular::gql::field("code"),
                    drogular::gql::field("title")
                })
        );
}

drogular::gql::Query ProjectTypeQueries::findById() {
    return drogular::gql::query("PortalProjectTypeById")
        .variable("id", "ID!")
        .select(
            drogular::gql::field("projectType")
                .arg("id", drogular::gql::variable("id"))
                .children({
                    drogular::gql::field("id"),
                    drogular::gql::field("code"),
                    drogular::gql::field("title")
                })
        );
}
