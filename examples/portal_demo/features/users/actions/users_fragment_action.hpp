#pragma once

#include "features/users/ui/portal_users_browser_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/action_renderer.hpp>
#include <drogular/component.hpp>

class PortalUsersFragmentComponent final
    : public drogular::TemplateComponent
{
public:
    std::string templatePath() const override {
        return "fragments/users_results.html";
    }
};

class PortalUsersFragmentAction final
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
            PortalUsersFragmentComponent
        >(
            context,
            [](drogular::RenderContext& renderContext) {
                PortalPageSupport::apply(renderContext, "users.title");
                PortalUsersBrowserSupport::apply(renderContext);
            }
        );
    }
};