#pragma once

#include "features/roles/ui/portal_roles_list_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/component.hpp>
#include <drogular/render_context.hpp>

class PortalRolesFragmentAction final
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
                "/roles?error=access_denied")
        ) {
            return *result;
        }

        drogular::RenderContext renderContext;
        renderContext.setServices(context.services());
        renderContext.setRequest(context.request());

        PortalPageSupport::apply(renderContext, "roles.page.title");
        PortalRolesListSupport::apply(renderContext);

        RolesListComponent component;
        component.onInit(renderContext);
        auto html = component.render(renderContext);
        component.onDestroy(renderContext);

        return drogular::ActionResult::html(std::move(html));
    }

private:
    class RolesListComponent final
        : public drogular::TemplateComponent
    {
    public:
        std::string templatePath() const override {
            return "fragments/roles_list.html";
        }
    };
};