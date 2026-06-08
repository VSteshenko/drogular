#include <drogular/graphql_client.hpp>

namespace drogular {

StaticGraphQLClient::StaticGraphQLClient(GraphQLResult result)
    : result_(std::move(result)) {
}

GraphQLResult StaticGraphQLClient::execute(const gql::Query&) {
    return result_;
}

} // namespace drogular
