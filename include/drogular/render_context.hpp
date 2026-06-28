#pragma once

#include <drogular/services.hpp>

#include <drogon/HttpRequest.h>

#include <any>
#include <optional>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <memory>
#include <typeindex>

namespace drogular {

class GraphQLClient;
class ApplicationServices;

namespace gql {
class Query;
}

/**
 * Thrown when a required render context value is missing or has a wrong type.
 */
class RenderContextError : public std::runtime_error {
public:
    explicit RenderContextError(const std::string& message);
};

/**
 * Stores GraphQL result data for rendering.
 */
class GraphQLResult {
public:
    template <typename T>
    void set(std::string key, T value) {
        values_[std::move(key)] = std::move(value);
    }

    template <typename T>
    std::optional<T> get(const std::string& key) const {
        const auto it = values_.find(key);

        if (it == values_.end()) {
            return std::nullopt;
        }

        const auto value = std::any_cast<T>(&it->second);

        if (value == nullptr) {
            return std::nullopt;
        }

        return *value;
    }

    template <typename T>
    T require(const std::string& key) const {
        const auto value = get<T>(key);

        if (!value.has_value()) {
            throw RenderContextError("GraphQL result value '" + key + "' is missing or has a wrong type");
        }

        return *value;
    }

    /**
     * Merges another GraphQL result into this one.
     *
     * Existing keys are replaced by incoming values.
    */
    void merge(GraphQLResult other);

    bool contains(const std::string& key) const;
    void clear();

private:
    std::unordered_map<std::string, std::any> values_;
};

/**
 * Provides data and services needed during rendering.
 */
class RenderContext {
public:
    RenderContext() = default;
    explicit RenderContext(const RenderContext* parent);

    /**
     * Returns application services.
     */
    ApplicationServices* services();

    /**
     * Returns read-only application services.
     */
    const ApplicationServices* services() const;

    template <typename T>
    std::shared_ptr<T> service() {
        if (services_ == nullptr) {
            return nullptr;
        }

        const auto type = std::type_index(typeid(T));

        const auto scopedIt = scopedServices_.find(type);

        if (scopedIt != scopedServices_.end()) {
            return std::static_pointer_cast<T>(scopedIt->second);
        }

        auto scoped = services_->createScoped<T>();

        if (scoped != nullptr) {
            scopedServices_[type] = scoped;
            return scoped;
        }

        return services_->service<T>();
    }

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

    template <typename T>
    std::shared_ptr<T> requireService() const {
        if (services_ == nullptr) {
            throw std::runtime_error(
                "ApplicationServices not set"
            );
        }

        return services_->requireService<T>();
    }

    /**
     * Creates a child render context.
     */
    RenderContext createChild() const;

    /**
     * Stores a typed value by key.
     */
    template <typename T>
    void set(std::string key, T value) {
        values_[std::move(key)] = std::move(value);
    }

    /**
     * Returns a typed value by key.
     *
     * Returns std::nullopt when the key is missing or the stored type is different.
     */
    template <typename T>
    std::optional<T> get(const std::string& key) const {
        const auto it = values_.find(key);

        if (it != values_.end()) {
            const auto value = std::any_cast<T>(&it->second);

            if (value != nullptr) {
                return *value;
            }

            return std::nullopt;
        }

        if (parent_ != nullptr) {
            return parent_->get<T>(key);
        }

        return std::nullopt;
    }

    /**
     * Checks whether the context contains a value for the given key.
     */
    bool contains(const std::string& key) const;

    /**
     * Removes a value from the context.
     */
    void remove(const std::string& key);

    /**
     * Removes all values from the context.
     */
    void clear();

    /**
     * Returns a required typed value by key.
     *
     * Throws RenderContextError when the key is missing or the stored type is different.
     */
    template <typename T>
    T require(const std::string& key) const {
        const auto value = get<T>(key);

        if (!value.has_value()) {
            throw RenderContextError("Render context value '" + key + "' is missing or has a wrong type");
        }

        return *value;
    }

    /**
     * Returns a typed value by key or the provided default value.
     */
    template <typename T>
    T getOr(const std::string& key, T defaultValue) const {
        const auto value = get<T>(key);

        if (!value.has_value()) {
            return defaultValue;
        }

        return *value;
    }

    /**
      * Sets the GraphQL client used by this render context.
     */
    void setGraphQLClient(GraphQLClient* client);

    /**
     * Returns true if a GraphQL client is available.
     */
    bool hasGraphQLClient() const;

    /**
     * Executes a GraphQL query and stores the result in this context.
     */
    void executeGraphQL(const gql::Query& query);

    /**
     * Sets application services for this render context.
     */
    void setServices(ApplicationServices* services);

    /**
     * Returns true if application services are available.
     */
    bool hasServices() const;

    /**
     * Returns mutable GraphQL result data.
     */
    GraphQLResult& graphql();

    /**
     * Returns read-only GraphQL result data.
     */
    const GraphQLResult& graphql() const;

    /**
     * Merges GraphQL data into the current context.
     */
    void mergeGraphQL(GraphQLResult result);

    void setRequest(
        const drogon::HttpRequestPtr& request
    );

    drogon::HttpRequestPtr request() const;

    std::optional<std::string> cookie(
        const std::string& name
    ) const;

    /**
     * Translates a key using the current request locale.
     *
     * Requires a TranslationProvider service to be registered.
     * If no provider is registered, the key itself is returned.
     */
    std::string translate(
        const std::string& key
    );

private:
    const RenderContext* parent_ = nullptr;
    std::unordered_map<std::string, std::any> values_;
    GraphQLClient* graphqlClient_ = nullptr;
    ApplicationServices* services_ = nullptr;
    std::unordered_map<std::type_index, std::shared_ptr<void>> scopedServices_;
    GraphQLResult graphql_;
    drogon::HttpRequestPtr request_;
};

} // namespace drogular