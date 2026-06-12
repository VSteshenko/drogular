#pragma once

#include <drogular/component_registry.hpp>
#include <drogular/dependency_graph.hpp>

#include <memory>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <functional>
#include <stdexcept>
#include <vector>
#include <string>

namespace drogular {

class GraphQLClient;

enum class ServiceLifetime {
    Singleton,
    LazySingleton,
    Transient,
    Scoped
};

class DependencyValidationResult {
public:
    void addError(std::string error) {
        errors_.push_back(std::move(error));
    }

    bool valid() const {
        return errors_.empty();
    }

    const std::vector<std::string>& errors() const {
        return errors_;
    }

private:
    std::vector<std::string> errors_;
};

class ApplicationServices {
public:
    void setGraphQLClient(std::shared_ptr<GraphQLClient> client);
    GraphQLClient* graphQLClient() const;

    /**
     * Registers an application service by type.
     */
    template <typename T>
    void registerService(std::shared_ptr<T> service) {
        services_[std::type_index(typeid(T))] = std::move(service);
    }

    /**
     * Returns an application service by type.
     */
    template <typename T>
    std::shared_ptr<T> service() const {
        const auto type = std::type_index(typeid(T));

        const auto serviceIt = services_.find(type);

        if (serviceIt == services_.end()) {
            return nullptr;
        }

        return std::static_pointer_cast<T>(serviceIt->second);
    }

    template <typename T>
    std::shared_ptr<T> service() {
        const auto type = std::type_index(typeid(T));

        const auto serviceIt = services_.find(type);

        if (serviceIt != services_.end()) {
            return std::static_pointer_cast<T>(serviceIt->second);
        }

        const auto factoryIt = factories_.find(type);

        if (factoryIt != factories_.end()) {
            auto service = factoryIt->second();
            services_[type] = service;

            return std::static_pointer_cast<T>(service);
        }

        const auto transientIt = transientFactories_.find(type);

        if (transientIt != transientFactories_.end()) {
            auto service = transientIt->second();

            return std::static_pointer_cast<T>(service);
        }

        return nullptr;
    }

    /**
     * Returns a registered service or throws.
     */
    template <typename T>
    std::shared_ptr<T> requireService() {
        auto resolved = service<T>();

        if (resolved == nullptr) {
            throw std::runtime_error(
                std::string("Service not registered: ") +
                typeid(T).name()
            );
        }

        return resolved;
    }

    /**
     * Returns mutable dependency graph.
     */
    DependencyGraph& dependencyGraph();

    /**
     * Returns read-only dependency graph.
     */
    const DependencyGraph& dependencyGraph() const;

    /**
     * Validates registered service dependencies.
     *
     * Checks that all dependencies recorded in the
     * dependency graph are registered in the
     * service container.
     *
     * Returns validation errors when required
     * dependencies are missing.
     */
    DependencyValidationResult validateDependencies() const;

    /**
     * Returns true when a service is registered.
     *
     * Checks singleton, lazy singleton,
     * transient and scoped registrations.
     */
    bool hasService(std::type_index type) const;

    /**
     * Creates and registers an application service by type.
     */
    template <typename T, typename... Args>
    std::shared_ptr<T> add(Args&&... args) {
        auto service = std::make_shared<T>(
            std::forward<Args>(args)...
        );

        registerService<T>(service);

        return service;
    }

    /**
     * Registers a service created by a factory.
     *
     * The factory is executed immediately.
     */
    template <typename T>
    std::shared_ptr<T> addFactory(
        std::function<std::shared_ptr<T>()> factory
    ) {
        auto service = factory();

        if (service == nullptr) {
            throw std::runtime_error("Service factory returned nullptr");
        }

        registerService<T>(service);

        return service;
    }

    template <typename T>
    std::shared_ptr<T> addFactory(
        ServiceLifetime lifetime,
        std::function<std::shared_ptr<T>()> factory
    ) {
        switch (lifetime) {
        case ServiceLifetime::Singleton:
            return addFactory<T>(std::move(factory));

        case ServiceLifetime::LazySingleton:
            addLazy<T>(std::move(factory));
            return nullptr;

        case ServiceLifetime::Transient:
            addTransient<T>(std::move(factory));
            return nullptr;

        case ServiceLifetime::Scoped:
            addScoped<T>(std::move(factory));
            return nullptr;
        }

        return nullptr;
    }

    template <typename T>
    void addLazy(
        std::function<std::shared_ptr<T>()> factory
    ) {
        factories_[std::type_index(typeid(T))] =
            [factory = std::move(factory)]() {
                auto service = factory();

                if (service == nullptr) {
                    throw std::runtime_error(
                        "Lazy service factory returned nullptr"
                    );
                }

                return std::static_pointer_cast<void>(service);
        };
    }

    template <typename T>
    void addTransient(
        std::function<std::shared_ptr<T>()> factory
    ) {
        transientFactories_[
            std::type_index(typeid(T))
        ] =
            [factory = std::move(factory)]() {
                auto service = factory();

                if (service == nullptr) {
                    throw std::runtime_error(
                        "Transient factory returned nullptr"
                    );
                }

                return std::static_pointer_cast<void>(
                    service
                );
        };
    }

    template <typename T>
    void addScoped(
        std::function<std::shared_ptr<T>()> factory
    ) {
        scopedFactories_[std::type_index(typeid(T))] =
            [factory = std::move(factory)]() {
                auto service = factory();

                if (service == nullptr) {
                    throw std::runtime_error(
                        "Scoped factory returned nullptr"
                    );
                }

                return std::static_pointer_cast<void>(service);
        };
    }

    template <typename T>
    std::shared_ptr<T> createScoped() {
        const auto type = std::type_index(typeid(T));

        const auto it = scopedFactories_.find(type);

        if (it == scopedFactories_.end()) {
            return nullptr;
        }

        return std::static_pointer_cast<T>(it->second());
    }

    template <typename T, typename... Args>
    std::shared_ptr<T> add(
        ServiceLifetime lifetime,
        Args&&... args
    ) {
        switch (lifetime) {
        case ServiceLifetime::Singleton:
            return add<T>(std::forward<Args>(args)...);

        case ServiceLifetime::LazySingleton:
            addLazy<T>(
                [args...]() {
                    return std::make_shared<T>(args...);
                }
            );
            return nullptr;

        case ServiceLifetime::Transient:
            addTransient<T>(
                [args...]() {
                    return std::make_shared<T>(args...);
                }
            );
            return nullptr;

        case ServiceLifetime::Scoped:
            addScoped<T>(
                [args...]() {
                    return std::make_shared<T>(args...);
                }
            );
            return nullptr;
        }

        return nullptr;
    }

    /**
     * Returns mutable component registry.
     */
    ComponentRegistry& components();

    /**
     * Returns read-only component registry.
     */
    const ComponentRegistry& components() const;

private:
    std::shared_ptr<GraphQLClient> graphQLClient_;
    std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> factories_;
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> transientFactories_;
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> scopedFactories_;
    DependencyGraph dependencyGraph_;
    ComponentRegistry componentRegistry_;
};

} // namespace drogular