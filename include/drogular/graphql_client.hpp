#pragma once

#include <drogular/component.hpp>
#include <drogular/graphql.hpp>
#include <drogular/graphql_request.hpp>
#include <drogular/graphql_response.hpp>
#include <drogular/graphql_variables.hpp>

#include <string>
#include <stdexcept>

namespace drogular {

/**
 * Thrown when GraphQL client execution fails.
 */
class GraphQLClientError : public std::runtime_error {
public:
    explicit GraphQLClientError(const std::string& message);
};

class GraphQLClient {
public:
    virtual ~GraphQLClient() = default;

    virtual GraphQLResponse execute(
        const gql::Query& query,
        const GraphQLVariables& variables = {}
    ) = 0;

    virtual GraphQLResponse execute(
        const gql::Mutation& mutation,
        const GraphQLVariables& variables = {}
    ) = 0;

    /**
     * Executes a GraphQL request.
     */
    virtual GraphQLResponse executeRequest(const GraphQLRequest& request) = 0;
};

class StaticGraphQLClient final : public GraphQLClient {
public:
    explicit StaticGraphQLClient(
        GraphQLResponse response
    );

    explicit StaticGraphQLClient(
        Json::Value data
    );

    GraphQLResponse execute(
        const gql::Query& query,
        const GraphQLVariables& variables = {}
    ) override;

    GraphQLResponse execute(
        const gql::Mutation& mutation,
        const GraphQLVariables& variables = {}
    ) override;

    GraphQLResponse executeRequest(
        const GraphQLRequest& request
    ) override;

private:
    GraphQLResponse response_;
};

class HttpGraphQLClient final : public GraphQLClient {
public:
    HttpGraphQLClient(
        std::string host,
        std::uint16_t port,
        std::string path = "/graphql"
    );

    GraphQLResponse execute(
        const gql::Query& query,
        const GraphQLVariables& variables = {}
    ) override {
        GraphQLRequest request(query.toString());
        request.variables(variables);

        auto response = executeRequest(request);
        throwIfGraphQLErrors(response);
        return response;
    }

    GraphQLResponse execute(
        const gql::Mutation& mutation,
        const GraphQLVariables& variables = {}
    ) override {
        GraphQLRequest request(mutation.toString());
        request.variables(variables);

        auto response = executeRequest(request);
        throwIfGraphQLErrors(response);
        return response;
    }

    GraphQLResponse executeRequest(const GraphQLRequest& request) override;

private:
    std::string host_;
    std::uint16_t port_;
    std::string path_;

    static void throwIfGraphQLErrors(
        const GraphQLResponse& response
    ) {
        if (response.hasErrors()) {
            throw GraphQLClientError(
                "GraphQL response contains errors"
            );
        }
    }
};

} // namespace drogular