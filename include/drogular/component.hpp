#pragma once

#include <drogular/template_cache.hpp>

#include <drogon//HttpRequest.h>

#include <any>
#include <optional>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <memory>
#include <vector>
#include <typeindex>

#include "services.hpp"

namespace drogular {

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

class GraphQLClient;
class ApplicationServices;

namespace gql {
class Query;
}

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

private:
    const RenderContext* parent_ = nullptr;
    std::unordered_map<std::string, std::any> values_;
    GraphQLClient* graphqlClient_ = nullptr;
    ApplicationServices* services_ = nullptr;
    std::unordered_map<std::type_index, std::shared_ptr<void>> scopedServices_;
    GraphQLResult graphql_;
    drogon::HttpRequestPtr request_;
};

/**
 * Base class for all UI components.
 *
 * Components are responsible for rendering reusable pieces of HTML.
 */
class Component {
public:
    virtual ~Component() = default;

    /**
     * Called before rendering.
     */
    virtual void onInit(RenderContext& context);

    /**
     * Called after component rendering.
     */
    virtual void onDestroy(RenderContext& context);

    /**
     * Renders the component into HTML.
     */
    virtual std::string render(RenderContext& context) = 0;

    /**
     * Returns child components.
     */
    virtual std::vector<std::shared_ptr<Component>> children();

    /**
     * Returns the slot name used by the parent component.
     */
    virtual std::string slot() const;

    /**
     * Stores a typed component parameter.
     */
    template <typename T>
    void setParam(std::string key, T value) {
        params_[std::move(key)] = std::move(value);
    }

    /**
     * Returns a typed component parameter.
     */
    template <typename T>
    std::optional<T> param(const std::string& key) const {
        const auto it = params_.find(key);

        if (it == params_.end()) {
            return std::nullopt;
        }

        const auto value = std::any_cast<T>(&it->second);

        if (value == nullptr) {
            return std::nullopt;
        }

        return *value;
    }

    /**
     * Copies component parameters into the render context.
     */
    void applyParams(RenderContext& context) const;

    /**
     * Stores a typed component input.
     */
    template <typename T>
    void setInput(std::string key, T value) {
        setParam(std::move(key), std::move(value));
    }

    /**
     * Returns a typed component input.
     */
    template <typename T>
    std::optional<T> input(const std::string& key) const {
        return param<T>(key);
    }

private:
    std::unordered_map<std::string, std::any> params_;
};

/**
 * Component base class for template-based rendering.
 */
class TemplateComponent : public Component {
public:
    /**
     * Returns template HTML.
     */
    virtual std::string templateHtml() const = 0;

    /**
     * Renders the template using RenderContext.
     */
    std::string render(RenderContext& context) override;

private:
    mutable template_compiler::TemplateCache templateCache_;
};

/**
 * Simple component that renders static HTML.
 */
class HtmlComponent final : public Component {
public:
    explicit HtmlComponent(std::string html);

    std::string render(RenderContext& context) override;

private:
    std::string html_;
};

} // namespace drogular