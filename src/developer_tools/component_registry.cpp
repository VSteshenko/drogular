#include <drogular/developer_tools/component_registry.hpp>

#include <stdexcept>
#include <utility>

namespace drogular {

void DeveloperToolsComponentRegistry::add(
    std::string name,
    std::string module
) {
    if (name.empty()) {
        throw std::invalid_argument(
            "Developer Tools component name must not be empty"
        );
    }

    if (module.empty()) {
        throw std::invalid_argument(
            "Developer Tools component module must not be empty"
        );
    }

    for (auto& entry : entries_) {
        if (entry.name == name) {
            entry.module = std::move(module);
            return;
        }
    }

    entries_.push_back({
        std::move(name),
        std::move(module)
    });
}

const DeveloperToolsComponentRegistration*
DeveloperToolsComponentRegistry::find(std::string_view name) const {
    for (const auto& entry : entries_) {
        if (entry.name == name) {
            return &entry;
        }
    }

    return nullptr;
}

const std::vector<DeveloperToolsComponentRegistration>& DeveloperToolsComponentRegistry::entries() const {
    return entries_;
}

Json::Value DeveloperToolsComponentRegistry::toJson() const {
    Json::Value root(Json::objectValue);
    Json::Value components(Json::arrayValue);

    for (const auto& entry : entries_) {
        Json::Value item(Json::objectValue);

        item["name"] = entry.name;
        item["module"] = entry.module;

        components.append(std::move(item));
    }

    root["components"] = std::move(components);

    return root;
}

} // namespace drogular