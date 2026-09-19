#pragma once

#include "features/department_members/ui/portal_department_members_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_handler.hpp>
#include <drogular/component.hpp>
#include <drogular/render_context.hpp>

class PortalDepartmentMembersInteractionSupport final {
public:
    static bool isInteraction(
        const drogular::ActionContext& context
    ) {
        const auto request = context.request();
        return request != nullptr &&
            request->getHeader("X-Drogular-Interaction") == "true";
    }

    static drogular::ActionResult render(
        drogular::ActionContext& context,
        int departmentId,
        const std::string& returnUrl,
        const std::string& error = {},
        const std::string& success = {},
        drogon::HttpStatusCode status = drogon::k200OK
    ) {
        drogular::RenderContext renderContext;
        renderContext.setServices(context.services());
        renderContext.setRequest(context.request());

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

        MembersComponent component;
        component.onInit(renderContext);
        auto html = component.render(renderContext);
        component.onDestroy(renderContext);

        return drogular::ActionResult::html(
            std::move(html),
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