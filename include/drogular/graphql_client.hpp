#pragma once

#include <drogular/component.hpp>
#include <drogular/graphql_request.hpp>
#include <drogular/graphql_response.hpp>

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

    virtual GraphQLResult execute(const gql::Query& query) = 0;

    /**
     * Executes a GraphQL request.
     */
    virtual GraphQLResponse executeRequest(const GraphQLRequest& request) = 0;
};

class StaticGraphQLClient final : public GraphQLClient {
public:
    explicit StaticGraphQLClient(GraphQLResult result);

    GraphQLResult execute(const gql::Query& query) override;

    GraphQLResponse executeRequest(const GraphQLRequest& request) override;

private:
    GraphQLResult result_;
};

class HttpGraphQLClient final : public GraphQLClient {
public:
    HttpGraphQLClient(
        std::string host,
        std::uint16_t port,
        std::string path = "/graphql"
    );

    GraphQLResult execute(const gql::Query& query) override;

    GraphQLResponse executeRequest(const GraphQLRequest& request) override;

private:
    std::string host_;
    std::uint16_t port_;
    std::string path_;
};

} // namespace drogular