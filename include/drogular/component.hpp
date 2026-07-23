#pragma once

#include <drogular/template_renderable.hpp>

#include <any>
#include <optional>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace drogular {

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
 *
 * TemplateComponent applies component inputs and then delegates the
 * common template pipeline to TemplateRenderable.
 */
class TemplateComponent : public Component, public TemplateRenderable {
public:
    /**
     * Renders the component through the shared template pipeline.
     */
    std::string render(RenderContext& context) override;
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