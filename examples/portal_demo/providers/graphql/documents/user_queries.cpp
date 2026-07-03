#include "user_queries.hpp"

drogular::gql::Query UserQueries::all() {
    return drogular::gql::query("PortalUsers")
        .select(
            drogular::gql::field("users")
                .children({
                    drogular::gql::field("username"),
                    drogular::gql::field("password"),
                    drogular::gql::field("role")
                })
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
                .children({
                    drogular::gql::field("username"),
                    drogular::gql::field("password"),
                    drogular::gql::field("role")
                })
        );
}