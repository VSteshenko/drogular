#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

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

private:
    std::shared_ptr<GraphQLClient> graphQLClient_;
    std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
};

} // namespace drogular