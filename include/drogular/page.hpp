#pragma once

#include <drogular/component.hpp>
#include <drogular/graphql.hpp>
#include <drogular/template_renderable.hpp>

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
 *
 * TemplatePage keeps the page-specific public API while delegating the
 * common template pipeline to TemplateRenderable.
 */
class TemplatePage : public Page, public TemplateRenderable {
public:
    /**
     * Renders the page through the shared template pipeline.
     */
    std::string render(RenderContext& context) override;
};

} // namespace drogular