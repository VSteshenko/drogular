#include "user_queries.hpp"
#include "../../../data/portal_graphql_selection_builder.hpp"
#include "../../../data/portal_schema.hpp"

drogular::gql::Query UserQueries::all() {
    return drogular::gql::query("PortalUsers")
        .select(
            drogular::gql::field("users")
            .children(
                PortalGraphQLSelectionBuilder::from(
                    PortalSchema::users()
                )
            )
        );
}

drogular::gql::Query UserQueries::findByCredentials() {
    return drogular::gql::query("PortalUserByCredentials")
        .variable("username", "String!")
        .variable("password", "String!")
        .select(
            drogular::gql::field("userByCredentials")
                .arg("username", drogular::gql::variable("username"))
                .arg("password", drogular::gql::variable("password"))
                .children(
                    PortalGraphQLSelectionBuilder::from(
                        PortalSchema::users()
                    )
                )
        );
}