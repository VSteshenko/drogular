#pragma once

#include <drogular/component.hpp>
#include <drogular/graphql.hpp>

#include <cstdint>
#include <string>

namespace drogular {

class GraphQLClient {
public:
    virtual ~GraphQLClient() = default;

    virtual GraphQLResult execute(const gql::Query& query) = 0;
};

class StaticGraphQLClient final : public GraphQLClient {
public:
    explicit StaticGraphQLClient(GraphQLResult result);

    GraphQLResult execute(const gql::Query& query) override;

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

private:
    std::string host_;
    std::uint16_t port_;
    std::string path_;
};

} // namespace drogular