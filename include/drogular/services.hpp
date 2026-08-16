#pragma once

#include <drogular/component_registry.hpp>
#include <drogular/dependency_graph.hpp>
#include <drogular/application_options.hpp>
#include <drogular/template_source_cache.hpp>

#include <memory>
#include <mutex>
#include <shared_mutex>
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

struct ServiceRegistration {
    std::string type;
    ServiceLifetime lifetime = ServiceLifetime::Singleton;
    bool instantiated = false;
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
        const auto type = std::type_index(typeid(T));
        std::unique_lock lock(servicesMutex_);
        services_[type] = std::move(service);
        serviceLifetimes_[type] = ServiceLifetime::Singleton;
    }

    /**
     * Returns an application service by type.
     */
    template <typename T>
    std::shared_ptr<T> service() const {
        const auto type = std::type_index(typeid(T));
        std::shared_lock lock(servicesMutex_);

        const auto serviceIt = services_.find(type);

        if (serviceIt == services_.end()) {
            return nullptr;
        }

        return std::static_pointer_cast<T>(serviceIt->second);
    }

    template <typename T>
    std::shared_ptr<T> service() {
        const auto type = std::type_index(typeid(T));
        std::function<std::shared_ptr<void>()> lazyFactory;
        std::shared_ptr<std::mutex> lazyMutex;
        std::function<std::shared_ptr<void>()> transientFactory;

        {
            std::shared_lock lock(servicesMutex_);

            const auto serviceIt = services_.find(type);

            if (serviceIt != services_.end()) {
                return std::static_pointer_cast<T>(serviceIt->second);
            }

            const auto factoryIt = factories_.find(type);

            if (factoryIt != factories_.end()) {
                lazyFactory = factoryIt->second;
                lazyMutex = lazyMutexes_.at(type);
            } else {
                const auto transientIt = transientFactories_.find(type);

                if (transientIt != transientFactories_.end()) {
                    transientFactory = transientIt->second;
                }
            }
        }

        if (lazyFactory) {
            std::lock_guard lazyLock(*lazyMutex);

            {
                std::shared_lock lock(servicesMutex_);
                const auto serviceIt = services_.find(type);

                if (serviceIt != services_.end()) {
                    return std::static_pointer_cast<T>(serviceIt->second);
                }
            }

            auto resolved = lazyFactory();

            {
                std::unique_lock lock(servicesMutex_);
                services_[type] = resolved;
            }

            return std::static_pointer_cast<T>(resolved);
        }

        if (transientFactory) {
            return std::static_pointer_cast<T>(transientFactory());
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
    void addLazy(std::function<std::shared_ptr<T>()> factory) {
        const auto type = std::type_index(typeid(T));
        std::unique_lock lock(servicesMutex_);
        serviceLifetimes_[type] = ServiceLifetime::LazySingleton;
        lazyMutexes_[type] = std::make_shared<std::mutex>();
        factories_[type] =
            wrapFactory<T>(
                std::move(factory),
                "Lazy service factory returned nullptr"
            );
    }

    template <typename T>
    void addTransient(std::function<std::shared_ptr<T>()> factory) {
        const auto type = std::type_index(typeid(T));
        std::unique_lock lock(servicesMutex_);

        serviceLifetimes_[type] =
            ServiceLifetime::Transient;
        transientFactories_[type] =
            wrapFactory<T>(
                std::move(factory),
                "Transient factory returned nullptr"
            );
    }

    template <typename T>
    void addScoped(std::function<std::shared_ptr<T>()> factory) {
        const auto type = std::type_index(typeid(T));
        std::unique_lock lock(servicesMutex_);

        serviceLifetimes_[type] =
            ServiceLifetime::Scoped;
        scopedFactories_[type] =
            wrapFactory<T>(
                std::move(factory),
                "Scoped factory returned nullptr"
            );
    }

    template <typename T>
    std::shared_ptr<T> createScoped() {
        const auto type = std::type_index(typeid(T));
        std::function<std::shared_ptr<void>()> factory;

        {
            std::shared_lock lock(servicesMutex_);
            const auto it = scopedFactories_.find(type);

            if (it == scopedFactories_.end()) {
                return nullptr;
            }

            factory = it->second;
        }

        return std::static_pointer_cast<T>(factory());
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

    std::vector<ServiceRegistration> registrations() const;

    void setOptions(ApplicationOptions* options) {
        options_ = options;

        if (options_ != nullptr) {
            templateSourceCache_.setLoader(
                TemplateLoader(
                    options_->templateRoot()
                )
            );
        }
    }

    ApplicationOptions* options() {
        return options_;
    }

    const ApplicationOptions* options() const {
        return options_;
    }

    TemplateSourceCache& templateSourceCache() {
        return templateSourceCache_;
    }

    const TemplateSourceCache& templateSourceCache() const {
        return templateSourceCache_;
    }

private:
    std::shared_ptr<GraphQLClient> graphQLClient_;
    std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> factories_;
    std::unordered_map<std::type_index, std::shared_ptr<std::mutex>> lazyMutexes_;
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> transientFactories_;
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> scopedFactories_;
    std::unordered_map<std::type_index, ServiceLifetime> serviceLifetimes_;
    DependencyGraph dependencyGraph_;
    ComponentRegistry componentRegistry_;
    ApplicationOptions* options_ = nullptr;
    TemplateSourceCache templateSourceCache_;
    mutable std::shared_mutex servicesMutex_;

    template <typename T>
    std::function<std::shared_ptr<void>()> wrapFactory(
        std::function<std::shared_ptr<T>()> factory,
        std::string errorMessage
    ) {
        return [factory = std::move(factory),
                errorMessage = std::move(errorMessage)]() {
            auto service = factory();

            if (service == nullptr) {
                throw std::runtime_error(errorMessage);
            }

            return std::static_pointer_cast<void>(service);
        };
    }
};

} // namespace drogular