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

DependencyGraph& ApplicationServices::dependencyGraph() {
    return dependencyGraph_;
}

const DependencyGraph& ApplicationServices::dependencyGraph() const {
    return dependencyGraph_;
}

ComponentRegistry& ApplicationServices::components() {
    return componentRegistry_;
}

const ComponentRegistry& ApplicationServices::components() const {
    return componentRegistry_;
}

} // namespace drogular
