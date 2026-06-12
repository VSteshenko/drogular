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

bool ApplicationServices::hasService(std::type_index type) const {
    return services_.contains(type) ||
           factories_.contains(type) ||
           transientFactories_.contains(type) ||
           scopedFactories_.contains(type);
}

DependencyValidationResult ApplicationServices::validateDependencies() const {
    DependencyValidationResult result;

    for (const auto& [serviceType, dependencies] :
         dependencyGraph_.allDependencies()) {
        for (const auto& dependencyType : dependencies) {
            if (!hasService(dependencyType)) {
                result.addError(
                    "Service dependency is not registered"
                );
            }
        }
         }

    if (dependencyGraph_.hasCircularDependencies()) {
        result.addError(
            "Circular dependency detected"
        );
    }

    return result;
}

ComponentRegistry& ApplicationServices::components() {
    return componentRegistry_;
}

const ComponentRegistry& ApplicationServices::components() const {
    return componentRegistry_;
}

} // namespace drogular
