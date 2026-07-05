#pragma once

#include <drogular/graphql_client.hpp>

#include <optional>
#include <vector>

namespace drogular {

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

} // namespace drogular
