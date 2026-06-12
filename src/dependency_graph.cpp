#include <drogular/dependency_graph.hpp>

#include <algorithm>
#include <unordered_set>

namespace drogular {

namespace {

bool findCycleFrom(
    std::type_index node,
    const std::unordered_map<
        std::type_index,
        std::vector<std::type_index>
    >& graph,
    std::unordered_set<std::type_index>& visiting,
    std::unordered_set<std::type_index>& visited,
    std::vector<std::type_index>& path
) {
    if (visited.contains(node)) {
        return false;
    }

    if (visiting.contains(node)) {
        path.push_back(node);
        return true;
    }

    visiting.insert(node);
    path.push_back(node);

    const auto it = graph.find(node);

    if (it != graph.end()) {
        for (const auto& dependency : it->second) {
            if (findCycleFrom(
                    dependency,
                    graph,
                    visiting,
                    visited,
                    path
                )) {
                return true;
            }
        }
    }

    visiting.erase(node);
    visited.insert(node);
    path.pop_back();

    return false;
}

bool hasCycleFrom(
    std::type_index node,
    const std::unordered_map<
        std::type_index,
        std::vector<std::type_index>
    >& graph,
    std::unordered_set<std::type_index>& visiting,
    std::unordered_set<std::type_index>& visited
) {
    if (visited.contains(node)) {
        return false;
    }

    if (visiting.contains(node)) {
        return true;
    }

    visiting.insert(node);

    const auto it = graph.find(node);

    if (it != graph.end()) {
        for (const auto& dependency : it->second) {
            if (hasCycleFrom(
                    dependency,
                    graph,
                    visiting,
                    visited
                )) {
                return true;
                }
        }
    }

    visiting.erase(node);
    visited.insert(node);

    return false;
}

} // namespace

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

const std::unordered_map<
    std::type_index,
    std::vector<std::type_index>
>& DependencyGraph::allDependencies() const {
    return dependencies_;
}

std::vector<std::type_index> DependencyGraph::circularDependencyPath() const {
    std::unordered_set<std::type_index> visiting;
    std::unordered_set<std::type_index> visited;
    std::vector<std::type_index> path;

    for (const auto& [service, _] : dependencies_) {
        if (findCycleFrom(
                service,
                dependencies_,
                visiting,
                visited,
                path
            )) {
            return path;
            }
    }

    return {};
}

bool DependencyGraph::hasCircularDependencies() const {
    std::unordered_set<std::type_index> visiting;
    std::unordered_set<std::type_index> visited;

    for (const auto& [service, _] : dependencies_) {
        if (hasCycleFrom(
                service,
                dependencies_,
                visiting,
                visited
            )) {
            return true;
        }
    }

    return false;
}

} // namespace drogular
