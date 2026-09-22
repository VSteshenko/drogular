#pragma once

#include "features/roles/providers/role_provider.hpp"
#include "features/roles/ui/portal_roles_list_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/action_renderer.hpp>
#include <drogular/component.hpp>

#include <cstdlib>

class PortalDeleteRoleAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        if (const auto result =
                drogular::ActionAuthSupport::
                    requireAuthentication(context)) {
            return *result;
        }

        if (const auto result =
                drogular::ActionAuthSupport::
                    requireSessionValue(
                        context,
                        "role",
                        "admin",
                        "/roles?error=access_denied"
                    )) {
            return *result;
        }

        const auto id =
            std::atoi(
                context.requireRouteParam("id").c_str()
            );

        auto roles =
            context.requireService<PortalRoleProvider>();

        if (!roles->remove(id)) {
            if (context.isInteraction()) {
                return renderList(
                    context,
                    "role_in_use",
                    "",
                    drogon::k422UnprocessableEntity
                );
            }

            return drogular::ActionResult::redirect(
                "/roles?error=role_in_use"
            );
        }

        if (context.isInteraction()) {
            return renderList(
                context,
                "",
                "role_deleted"
            );
        }

        return drogular::ActionResult::redirect(
            "/roles?success=role_deleted"
        );
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

    static drogular::ActionResult renderList(
        drogular::ActionContext& context,
        const std::string& error,
        const std::string& success,
        drogon::HttpStatusCode status = drogon::k200OK
    ) {
        return drogular::ActionRenderer::render<
            RolesListComponent
        >(
            context,
            [&](drogular::RenderContext& renderContext) {
                PortalPageSupport::apply(
                    renderContext,
                    "roles.page.title"
                );
                PortalRolesListSupport::apply(
                    renderContext,
                    error,
                    success
                );
            },
            status
        );
    }
};