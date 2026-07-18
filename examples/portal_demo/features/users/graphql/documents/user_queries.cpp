#include "user_queries.hpp"
#include "data/portal_graphql_document_builder.hpp"
#include "data/portal_schema.hpp"

drogular::gql::Query UserQueries::all() {
    return PortalGraphQLDocumentBuilder::all(
        "PortalUsers",
        "users",
        PortalSchema::users()
    );
}

drogular::gql::Query UserQueries::search(
    const PortalUserQuery&
) {
    return drogular::gql::query("SearchPortalUsers")
        .variable(
            "search",
            "String"
        )
        .variable(
            "role",
            "String"
        )
        .variable(
            "sorting",
            "[UserSortInput!]"
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
            drogular::gql::field("userPage")
                .arg(
                    "search",
                    drogular::gql::variable("search")
                )
                .arg(
                    "role",
                    drogular::gql::variable("role")
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
                                PortalSchema::users()
                            )
                        ),
                    drogular::gql::field("page"),
                    drogular::gql::field("pageSize"),
                    drogular::gql::field("totalItems"),
                    drogular::gql::field("totalPages")
                })
        );
}

drogular::gql::Query UserQueries::findByCredentials() {
    return drogular::gql::query("PortalUserByCredentials")
        .variable(
            "username",
            "String!"
        )
        .variable(
            "password",
            "String!"
        )
        .select(
            drogular::gql::field("userByCredentials")
                .arg(
                    "username",
                    drogular::gql::variable("username")
                )
                .arg(
                    "password",
                    drogular::gql::variable("password")
                )
                .children(
                    PortalGraphQLSelectionBuilder::from(
                        PortalSchema::users()
                    )
                )
        );
}