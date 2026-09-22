#pragma once

#include "features/project_types/providers/project_type_provider.hpp"
#include "features/project_types/ui/portal_project_types_list_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/action_renderer.hpp>
#include <drogular/component.hpp>

#include <cstdlib>
#include <string>

class PortalDeleteProjectTypeAction final
    : public drogular::ActionHandler
{
public:
    drogular::ActionResult handle(
        drogular::ActionContext& context
    ) override {
        if (const auto result =
                drogular::ActionAuthSupport::requireAuthentication(
                    context
                )) {
            return *result;
        }

        if (const auto result =
                drogular::ActionAuthSupport::requireSessionValue(
                    context,
                    "role",
                    "admin",
                    "/project-types?error=access_denied"
                )) {
            return *result;
                }

        const auto id =
            std::atoi(
                context.requireRouteParam("id").c_str()
            );

        auto projectTypes =
            context.requireService<
                PortalProjectTypeProvider
            >();

        if (!projectTypes->remove(id)) {
            if (context.isInteraction()) {
                return renderList(
                    context,
                    "project_type_in_use",
                    "",
                    drogon::k422UnprocessableEntity
                );
            }

            return drogular::ActionResult::redirect(
                "/project-types?error=project_type_in_use"
            );
        }

        if (context.isInteraction()) {
            return renderList(
                context,
                "",
                "project_type_deleted"
            );
        }

        return drogular::ActionResult::redirect(
            "/project-types?success=project_type_deleted"
        );
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

    static drogular::ActionResult renderList(
        drogular::ActionContext& context,
        const std::string& error,
        const std::string& success,
        drogon::HttpStatusCode status = drogon::k200OK
    ) {
        return drogular::ActionRenderer::render<
            ProjectTypesListComponent
        >(
            context,
            [&](drogular::RenderContext& renderContext) {
                PortalPageSupport::apply(
                    renderContext,
                    "project_types.page.title"
                );
                PortalProjectTypesListSupport::apply(
                    renderContext,
                    error,
                    success
                );
            },
            status
        );
    }
};