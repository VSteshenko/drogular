#pragma once

#include "features/departments/ui/portal_departments_browser_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/component.hpp>
#include <drogular/page_auth_support.hpp>
#include <drogular/render_context.hpp>

class PortalDepartmentsFragmentComponent final
    : public drogular::TemplateComponent
{
public:
    std::string templatePath() const override {
        return "fragments/departments_results.html";
    }
};

class PortalDepartmentsFragmentAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        drogular::RenderContext renderContext;
        renderContext.setServices(context.services());
        renderContext.setRequest(context.request());

        PortalPageSupport::apply(renderContext, "departments.title");
        if (!drogular::PageAuthSupport::requireAuthentication(renderContext)) {
            return drogular::ActionResult::html(
                "<div class=\"dg-empty-state\">Authentication required.</div>"
            );
        }

        PortalDepartmentsBrowserSupport::apply(renderContext);

        PortalDepartmentsFragmentComponent component;
        component.onInit(renderContext);
        auto html = component.render(renderContext);
        component.onDestroy(renderContext);

        return drogular::ActionResult::html(std::move(html));
    }
};