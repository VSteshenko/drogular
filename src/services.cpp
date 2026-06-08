#include <drogular/services.hpp>

#include <drogular/graphql_client.hpp>

namespace drogular {

void ApplicationServices::setGraphQLClient(
    std::shared_ptr<GraphQLClient> client
) {
    graphQLClient_ = std::move(client);
}

GraphQLClient* ApplicationServices::graphQLClient() const {
    return graphQLClient_.get();
}

} // namespace drogular
