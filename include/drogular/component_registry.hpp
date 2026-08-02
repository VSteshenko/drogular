#pragma once

#include <drogular/diagnostics.hpp>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace drogular {

class Component;

/**
 * Creates components dynamically by tag name.
 */
class ComponentRegistry {
public:
    /**
     * Registers a component type by tag name.
     */
    template <typename ComponentType>
    void registerComponent(std::string tag) {
        if (factories_.contains(tag)) {
            diagnostics_.warning(
                "DGL-CMP-001",
                "Component tag is already registered: " + tag
            );
        }

        factories_[std::move(tag)] = []() {
            return std::make_shared<ComponentType>();
        };
    }

    /**
     * Registers a component type using ComponentType::tag.
     */
    template <typename ComponentType>
    void registerComponent() {
        registerComponent<ComponentType>(
            ComponentType::tag
        );
    }

    /**
     * Creates a component by tag name.
     *
     * Returns nullptr when the component is not registered.
     */
    std::shared_ptr<Component> create(const std::string& tag) const;

    /**
     * Returns true if a component tag is registered.
     */
    bool contains(const std::string& tag) const;

    /**
     * Returns diagnostics produced while registering components.
     */
    const Diagnostics& diagnostics() const;

    /**
     * Clears registration diagnostics.
     */
    void clearDiagnostics();

private:
    std::unordered_map<
        std::string,
        std::function<std::shared_ptr<Component>()>
    > factories_;
    Diagnostics diagnostics_;
};

} // namespace drogular
