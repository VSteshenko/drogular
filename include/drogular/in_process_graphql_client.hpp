#pragma once

#include <drogular/graphql_client.hpp>
#include <drogular/graphql_execution_context.hpp>
#include <drogular/graphql_server.hpp>

#include <functional>
#include <memory>

namespace drogular {

/**
 * Executes GraphQL operations directly against a GraphQLServer.
 *
 * This client preserves the GraphQLClient abstraction without using HTTP,
 * which makes it suitable for modular applications and integration tests.
 */
class InProcessGraphQLClient final : public GraphQLClient {
public:
    using ContextFactory = std::function<GraphQLExecutionContext()>;

    explicit InProcessGraphQLClient(
        std::shared_ptr<GraphQLServer> server,
        ContextFactory contextFactory = {}
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
    GraphQLExecutionContext createContext() const;

    std::shared_ptr<GraphQLServer> server_;
    ContextFactory contextFactory_;
};

} // namespace drogular