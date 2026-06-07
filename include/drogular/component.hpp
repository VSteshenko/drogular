#pragma once

#include <any>
#include <optional>
#include <string>
#include <unordered_map>

namespace drogular {

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

private:
    std::unordered_map<std::string, std::any> values_;
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
};

} // namespace drogular