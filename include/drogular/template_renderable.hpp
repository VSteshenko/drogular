#pragma once

#include <drogular/template_cache.hpp>

#include <string>

namespace drogular {

class RenderContext;

/**
 * Shared rendering contract for template-backed pages and components.
 *
 * TemplateRenderable owns the common template pipeline:
 * source loading, optional layout composition, preprocessing,
 * AST compilation/caching, rendering, and nested component expansion.
 */
class TemplateRenderable {
public:
    virtual ~TemplateRenderable() = default;

    /**
     * Returns inline template HTML.
     *
     * Used when templatePath() is empty.
     */
    virtual std::string templateHtml() const {
        return "";
    }

    /**
     * Returns the external template file path.
     *
     * Empty path means templateHtml() will be used.
     */
    virtual std::string templatePath() const {
        return "";
    }

    /**
     * Returns the optional external layout file path.
     *
     * The default empty path renders the template without a layout.
     */
    virtual std::string layoutPath() const {
        return "";
    }

protected:
    /**
     * Executes the complete template rendering pipeline.
     */
    std::string renderTemplate(RenderContext& context) const;

private:
    mutable template_compiler::TemplateCache templateCache_;
};

} // namespace drogular