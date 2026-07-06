#include "user_mutations.hpp"
#include "../../../data/portal_graphql_selection_builder.hpp"
#include "../../../data/portal_schema.hpp"

drogular::gql::Mutation UserMutations::create(
    const PortalUser&
) {
    return drogular::gql::mutation("CreatePortalUser")
        .variable("user", "UserInput!")
        .select(
            drogular::gql::field("createUser")
                .arg("user", drogular::gql::variable("user"))
                .children(
                    PortalGraphQLSelectionBuilder::from(
                        PortalSchema::users()
                    )
                )
        );
}

drogular::gql::Mutation UserMutations::update(
    const PortalUser&
) {
    return drogular::gql::mutation("UpdatePortalUser")
        .variable("user", "UserInput!")
        .select(
            drogular::gql::field("updateUser")
                .arg("user", drogular::gql::variable("user"))
                .children(
                    PortalGraphQLSelectionBuilder::from(
                        PortalSchema::users()
                    )
                )
        );
}