#include "user_mutations.hpp"
#include "data/portal_graphql_document_builder.hpp"
#include "data/portal_schema.hpp"
#include "features/users/data/portal_user_update.hpp"

drogular::gql::Mutation UserMutations::create(
    const PortalUser&
) {
    return PortalGraphQLDocumentBuilder::mutation(
        "CreatePortalUser",
        "createUser",
        "user",
        "UserInput!",
        PortalSchema::users()
    );
}

drogular::gql::Mutation UserMutations::update(
    const PortalUserUpdate&
) {
    return PortalGraphQLDocumentBuilder::mutation(
        "UpdatePortalUser",
        "updateUser",
        "user",
        "UserUpdateInput!",
        PortalSchema::users()
    );
}