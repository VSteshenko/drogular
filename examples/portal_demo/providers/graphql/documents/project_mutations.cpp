#include "project_mutations.hpp"
#include "../../../data/portal_graphql_selection_builder.hpp"
#include "../../../data/portal_schema.hpp"

drogular::gql::Mutation ProjectMutations::create(
    const PortalProject&
) {
    return drogular::gql::mutation("CreatePortalProject")
        .variable("project", "ProjectInput!")
        .select(
            drogular::gql::field("createProject")
                .arg("project", drogular::gql::variable("project"))
                .children(
                    PortalGraphQLSelectionBuilder::from(
                        PortalSchema::projects()
                    )
                )
        );
}

drogular::gql::Mutation ProjectMutations::update(
    const PortalProject&
) {
    return drogular::gql::mutation("UpdatePortalProject")
        .variable("project", "ProjectInput!")
        .select(
            drogular::gql::field("updateProject")
                .arg("project", drogular::gql::variable("project"))
                .children(
                    PortalGraphQLSelectionBuilder::from(
                        PortalSchema::projects()
                    )
                )
        );
}

drogular::gql::Mutation ProjectMutations::remove(
    int
) {
    return drogular::gql::mutation("RemovePortalProject")
        .variable("id", "ID!")
        .select(
            drogular::gql::field("removeProject")
                .arg("id", drogular::gql::variable("id"))
        );
}