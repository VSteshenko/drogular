#pragma once

#include <drogular/component.hpp>
#include <drogular/graphql.hpp>
#include <drogular/template_cache.hpp>

#include <optional>

namespace drogular {

/**
 * Base class for full pages.
 *
 * A page is a component that can be mounted to a route.
 */
class Page : public Component {
public:
    ~Page() override = default;

    /**
     * Returns the GraphQL query required by this page.
     */
    virtual std::optional<gql::Query> query() const;
};

/**
 * Page base class for template-based rendering.
 */
class TemplatePage : public Page {
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

} // namespace drogular
