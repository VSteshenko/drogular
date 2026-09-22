#pragma once

#include "features/departments/ui/portal_departments_browser_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/action_renderer.hpp>
#include <drogular/component.hpp>

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
        if (const auto result =
                drogular::ActionAuthSupport::requireAuthentication(context)) {
            return *result;
        }

        return drogular::ActionRenderer::render<
            PortalDepartmentsFragmentComponent
        >(
            context,
            [](drogular::RenderContext& renderContext) {
                PortalPageSupport::apply(
                    renderContext,
                    "departments.title"
                );
                PortalDepartmentsBrowserSupport::apply(renderContext);
            }
        );
    }
};