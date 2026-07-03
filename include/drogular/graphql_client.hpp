#pragma once

#include <drogular/component.hpp>
#include <drogular/graphql.hpp>
#include <drogular/graphql_request.hpp>
#include <drogular/graphql_response.hpp>
#include <drogular/graphql_variables.hpp>

#include <string>
#include <stdexcept>
#include <optional>
#include <vector>

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

    /**
     * Executes a GraphQL query.
     */
    virtual GraphQLResponse execute(
        const gql::Query& query,
        const GraphQLVariables& variables = {}
    ) = 0;

    /**
     * Executes a GraphQL mutation.
     */
    virtual GraphQLResponse execute(
        const gql::Mutation& mutation,
        const GraphQLVariables& variables = {}
    ) = 0;

    /**
     * Executes a GraphQL request.
     */
    virtual GraphQLResponse executeRequest(const GraphQLRequest& request) = 0;
};

class StaticGraphQLClient : public GraphQLClient {
public:
    /**
     * Creates a client returning the specified GraphQL response.
     */
    explicit StaticGraphQLClient(
        Json::Value data
    );

    /**
     * Executes a GraphQL query.
     */
    GraphQLResponse execute(
        const gql::Query& query,
        const GraphQLVariables& variables = {}
    ) override;

    /**
     * Executes a GraphQL mutation.
     */
    GraphQLResponse execute(
        const gql::Mutation& mutation,
        const GraphQLVariables& variables = {}
    ) override;

    /**
     * Executes a raw GraphQL request.
     */
    GraphQLResponse executeRequest(
        const GraphQLRequest& request
    ) override;

    /**
     * Returns all executed GraphQL requests.
     *
     * Requests are stored in execution order and can be used
     * to verify generated queries and variables in tests.
     */
    const std::vector<GraphQLRequest>& requests() const;

    /**
     * Returns the last executed GraphQL request.
     */
    std::optional<GraphQLRequest> lastRequest() const;

    /**
     * Removes all stored requests.
     */
    void clearRequests();

    /**
     * Returns the number of executed requests.
     */
    std::size_t requestCount() const;

private:
    GraphQLResponse response_;
    std::vector<GraphQLRequest> requests_;
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