#include "project_type_mutations.hpp"

#include "../../../data/portal_graphql_document_builder.hpp"
#include "../../../data/portal_schema.hpp"

drogular::gql::Mutation ProjectTypeMutations::create(
    const PortalProjectTypeCreate&
) {
    return PortalGraphQLDocumentBuilder::mutation(
        "CreatePortalProjectType",
        "createProjectType",
        "projectType",
        "ProjectTypeCreateInput!",
        PortalSchema::projectTypes()
    );
}

drogular::gql::Mutation ProjectTypeMutations::update(
    const PortalProjectTypeUpdate&
) {
    return PortalGraphQLDocumentBuilder::mutation(
        "UpdatePortalProjectType",
        "updateProjectType",
        "projectType",
        "ProjectTypeUpdateInput!",
        PortalSchema::projectTypes()
    );
}

drogular::gql::Mutation ProjectTypeMutations::remove() {
    return drogular::gql::mutation(
        "RemovePortalProjectType"
    )
        .variable(
            "id",
            "ID!"
        )
        .select(
            drogular::gql::field(
                "removeProjectType"
            )
                .arg(
                    "id",
                    drogular::gql::variable("id")
                )
        );
}