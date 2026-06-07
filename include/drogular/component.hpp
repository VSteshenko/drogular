#pragma once

#include <any>
#include <optional>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <memory>
#include <vector>

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

        if (it == values_.end()) {
            return std::nullopt;
        }

        const auto value = std::any_cast<T>(&it->second);

        if (value == nullptr) {
            return std::nullopt;
        }

        return *value;
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
     * Returns mutable GraphQL result data.
     */
    GraphQLResult& graphql();

    /**
     * Returns read-only GraphQL result data.
     */
    const GraphQLResult& graphql() const;

private:
    std::unordered_map<std::string, std::any> values_;
    GraphQLResult graphql_;
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
     * Renders the component into HTML.
     */
    virtual std::string render(RenderContext& context) = 0;

    /**
     * Returns child components.
     */
    virtual std::vector<std::shared_ptr<Component>> children();
};

} // namespace drogular