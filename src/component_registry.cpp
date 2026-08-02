#include <drogular/component_registry.hpp>
#include <drogular/component.hpp>
#include <algorithm>

namespace drogular {

std::shared_ptr<Component> ComponentRegistry::create(
    const std::string& tag
) const {
    const auto it = factories_.find(tag);

    if (it == factories_.end()) {
        return nullptr;
    }

    return it->second();
}

bool ComponentRegistry::contains(const std::string& tag) const {
    return factories_.contains(tag);
}

const Diagnostics& ComponentRegistry::diagnostics() const {
    return diagnostics_;
}

void ComponentRegistry::clearDiagnostics() {
    diagnostics_.clear();
}

std::vector<std::string> ComponentRegistry::tags() const {
    std::vector<std::string> result;
    result.reserve(factories_.size());

    for (const auto& [tag, _] : factories_) {
        result.push_back(tag);
    }

    std::sort(
        result.begin(),
        result.end()
    );

    return result;
}

} // namespace drogular