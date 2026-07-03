#include "user_mutations.hpp"

drogular::gql::Mutation UserMutations::create(
    const PortalUser&
) {
    return drogular::gql::mutation("CreatePortalUser")
        .variable("user", "UserInput!")
        .select(
            drogular::gql::field("createUser")
                .arg("user", drogular::gql::variable("user"))
                .children({
                    drogular::gql::field("username"),
                    drogular::gql::field("password"),
                    drogular::gql::field("role")
                })
        );
}