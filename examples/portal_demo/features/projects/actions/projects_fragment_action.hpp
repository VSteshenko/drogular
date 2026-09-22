#pragma once

#include "features/projects/ui/portal_projects_browser_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/action_renderer.hpp>
#include <drogular/component.hpp>

class PortalProjectsFragmentComponent final : public drogular::TemplateComponent {
public:
    std::string templatePath() const override {
        return "fragments/projects_results.html";
    }
};

class PortalProjectsFragmentAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        if (const auto result =
                drogular::ActionAuthSupport::requireAuthentication(context)) {
            return *result;
        }

        return drogular::ActionRenderer::render<
            PortalProjectsFragmentComponent
        >(
            context,
            [](drogular::RenderContext& renderContext) {
                PortalPageSupport::apply(
                    renderContext,
                    "projects.title"
                );
                PortalProjectsBrowserSupport::apply(renderContext);
            }
        );
    }
};