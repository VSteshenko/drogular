#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <functional>
#include <stdexcept>

namespace drogular {

class GraphQLClient;

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

        if (factoryIt == factories_.end()) {
            return nullptr;
        }

        auto service = factoryIt->second();

        services_[type] = service;

        return std::static_pointer_cast<T>(service);
    }

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

private:
    std::shared_ptr<GraphQLClient> graphQLClient_;
    std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> factories_;
};

} // namespace drogular