#include <drogular/dependency_graph.hpp>

#include <algorithm>

namespace drogular {

std::vector<std::type_index> DependencyGraph::dependencies(
    std::type_index service
) const {
    const auto it = dependencies_.find(service);

    if (it == dependencies_.end()) {
        return {};
    }

    return it->second;
}

bool DependencyGraph::dependsOn(
    std::type_index service,
    std::type_index dependency
) const {
    const auto serviceDependencies = dependencies(service);

    return std::find(
        serviceDependencies.begin(),
        serviceDependencies.end(),
        dependency
    ) != serviceDependencies.end();
}

} // namespace drogular
