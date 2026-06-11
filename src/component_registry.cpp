#include <drogular/component_registry.hpp>
#include <drogular/component.hpp>

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

} // namespace drogular