#pragma once

#include <typeindex>
#include <unordered_map>
#include <vector>

namespace drogular {

/**
 * Stores service dependency metadata.
 */
class DependencyGraph {
public:
    template <typename ServiceType, typename DependencyType>
    void addDependency() {
        dependencies_[std::type_index(typeid(ServiceType))]
            .push_back(std::type_index(typeid(DependencyType)));
    }

    template <typename ServiceType>
    std::vector<std::type_index> dependencies() const {
        return dependencies(std::type_index(typeid(ServiceType)));
    }

    std::vector<std::type_index> dependencies(
        std::type_index service
    ) const;

    bool dependsOn(
        std::type_index service,
        std::type_index dependency
    ) const;

    const std::unordered_map<
        std::type_index,
        std::vector<std::type_index>
    >& allDependencies() const;

private:
    std::unordered_map<
        std::type_index,
        std::vector<std::type_index>
    > dependencies_;
};

} // namespace drogular
