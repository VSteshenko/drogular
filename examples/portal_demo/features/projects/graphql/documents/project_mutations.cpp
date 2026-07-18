#include "features/projects/graphql/documents/project_mutations.hpp"
#include "data/portal_graphql_document_builder.hpp"
#include "data/portal_schema.hpp"
#include "features/projects/data/portal_project_update.hpp"

drogular::gql::Mutation ProjectMutations::create(
    const PortalProject&
) {
    return PortalGraphQLDocumentBuilder::mutation(
        "CreatePortalProject",
        "createProject",
        "project",
        "ProjectInput!",
        PortalSchema::projects()
    );
}

drogular::gql::Mutation ProjectMutations::update(
    const PortalProjectUpdate&
) {
    return PortalGraphQLDocumentBuilder::mutation(
        "UpdatePortalProject",
        "updateProject",
        "project",
        "ProjectUpdateInput!",
        PortalSchema::projects()
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