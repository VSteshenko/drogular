#pragma once

#include "features/project_types/ui/portal_project_types_list_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/component.hpp>
#include <drogular/render_context.hpp>

class PortalProjectTypesFragmentAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        if (const auto result =
            drogular::ActionAuthSupport::requireAuthentication(context)
        ) {
            return *result;
        }

        if (const auto result =
            drogular::ActionAuthSupport::requireSessionValue(
                context,
                "role",
                "admin",
                "/project-types?error=access_denied")
        ) {
            return *result;
        }

        drogular::RenderContext renderContext;
        renderContext.setServices(context.services());
        renderContext.setRequest(context.request());

        PortalPageSupport::apply(
            renderContext,
            "project_types.page.title"
        );
        PortalProjectTypesListSupport::apply(renderContext);

        ProjectTypesListComponent component;
        component.onInit(renderContext);
        auto html = component.render(renderContext);
        component.onDestroy(renderContext);

        return drogular::ActionResult::html(std::move(html));
    }

private:
    class ProjectTypesListComponent final
        : public drogular::TemplateComponent
    {
    public:
        std::string templatePath() const override {
            return "fragments/project_types_list.html";
        }
    };
};