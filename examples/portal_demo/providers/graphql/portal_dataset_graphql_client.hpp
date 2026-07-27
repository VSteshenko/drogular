#pragma once

#include "core/graphql/server/portal_graphql_server.hpp"

#include <drogular/static_graphql_client.hpp>

#include <memory>
#include <string>
#include <utility>

class PortalDatasetGraphQLClient final : public drogular::GraphQLClient {
public:
    explicit PortalDatasetGraphQLClient(
        std::shared_ptr<PortalGraphQLServer> server
    )
        : server_(std::move(server))
    {
    }

    drogular::GraphQLResponse execute(
        const drogular::gql::Query& query,
        const drogular::GraphQLVariables& variables = {}
    ) override {
        return server_->executeQuery(
            operationName(query.toString(), "query"),
            variables
        );
    }

    drogular::GraphQLResponse execute(
        const drogular::gql::Mutation& mutation,
        const drogular::GraphQLVariables& variables = {}
    ) override {
        return server_->executeMutation(
            operationName(mutation.toString(), "mutation"),
            variables
        );
    }

    drogular::GraphQLResponse executeRequest(
        const drogular::GraphQLRequest&
    ) override {
        return server_->executeQuery("", {});
    }

    static std::string operationName(
        const std::string& text,
        const std::string& type
    ) {
        const auto prefix = type + " ";
        if (!text.starts_with(prefix)) {
            return "";
        }

        const auto start = prefix.size();
        const auto end = text.find_first_of(" (", start);

        return end == std::string::npos
            ? text.substr(start)
            : text.substr(start, end - start);
    }

private:
    std::shared_ptr<PortalGraphQLServer> server_;
};