#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>

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
        const auto it = services_.find(std::type_index(typeid(T)));

        if (it == services_.end()) {
            return nullptr;
        }

        return std::static_pointer_cast<T>(it->second);
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

private:
    std::shared_ptr<GraphQLClient> graphQLClient_;
    std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
};

} // namespace drogular