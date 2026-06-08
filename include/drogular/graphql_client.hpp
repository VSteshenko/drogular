#pragma once

#include <drogular/component.hpp>
#include <drogular/graphql.hpp>

namespace drogular {

/**
 * Executes GraphQL queries and returns GraphQL result data.
 */
class GraphQLClient {
public:
    virtual ~GraphQLClient() = default;

    /**
     * Executes a GraphQL query.
     */
    virtual GraphQLResult execute(const gql::Query& query) = 0;
};

/**
 * Simple GraphQL client for tests and examples.
 *
 * It always returns the same predefined result.
 */
class StaticGraphQLClient final : public GraphQLClient {
public:
    explicit StaticGraphQLClient(GraphQLResult result);

    GraphQLResult execute(const gql::Query& query) override;

private:
    GraphQLResult result_;
};

} // namespace drogular
