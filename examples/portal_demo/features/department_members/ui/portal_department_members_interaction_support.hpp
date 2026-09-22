#pragma once

#include "features/department_members/ui/portal_department_members_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/action_renderer.hpp>
#include <drogular/component.hpp>

class PortalDepartmentMembersInteractionSupport final {
public:
    static drogular::ActionResult render(
        drogular::ActionContext& context,
        int departmentId,
        const std::string& returnUrl,
        const std::string& error = {},
        const std::string& success = {},
        drogon::HttpStatusCode status = drogon::k200OK
    ) {
        return drogular::ActionRenderer::render<
            MembersComponent
        >(
            context,
            [&](drogular::RenderContext& renderContext) {
                PortalPageSupport::apply(
                    renderContext,
                    "departments.details.title"
                );
                renderContext.set("departmentId", departmentId);
                PortalDepartmentMembersSupport::apply(
                    renderContext,
                    departmentId,
                    returnUrl,
                    error,
                    success
                );
            },
            status
        );
    }

private:
    class MembersComponent final
        : public drogular::TemplateComponent
    {
    public:
        std::string templatePath() const override {
            return "fragments/department_members.html";
        }
    };
};