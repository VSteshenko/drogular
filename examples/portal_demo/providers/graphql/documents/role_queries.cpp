#include "role_queries.hpp"
#include "../../../data/portal_graphql_selection_builder.hpp"
#include "../../../data/portal_schema.hpp"

#include <drogular/graphql.hpp>

drogular::gql::Query RoleQueries::all() {
    return drogular::gql::query("PortalRoles")
        .select(
            drogular::gql::field("role")
            .children(
                PortalGraphQLSelectionBuilder::from(
                    PortalSchema::roles()
                )
            )
        );
}

drogular::gql::Query RoleQueries::findById(
    int id
) {
    return drogular::gql::query("PortalRoleById")
        .variable("id", "ID!")
        .select(
            drogular::gql::field("role")
                .arg(
                    "id",
                    drogular::gql::variable("id")
                )
                .children(
                    PortalGraphQLSelectionBuilder::from(
                        PortalSchema::roles()
                    )
                )
        );
}

drogular::gql::Query RoleQueries::findByCode(
    const std::string& code
) {
    return drogular::gql::query("PortalRoleByCode")
        .variable("code", "CODE!")
        .select(
            drogular::gql::field("role")
                .arg(
                    "code",
                    drogular::gql::variable("code")
                )
                .children(
                    PortalGraphQLSelectionBuilder::from(
                        PortalSchema::roles()
                    )
                )
        );
}