#pragma once

#include "features/dashboard/ui/portal_dashboard_tasks_support.hpp"
#include "ui/portal_page_support.hpp"

#include <drogular/action_auth_support.hpp>
#include <drogular/action_handler.hpp>
#include <drogular/action_renderer.hpp>
#include <drogular/component.hpp>

#include <charconv>
#include <string>
#include <string_view>

class PortalDashboardTaskActionSupport final {
public:
    template <typename Value, typename Parse, typename Update>
    static drogular::ActionResult handle(
        drogular::ActionContext& context,
        const char* formName,
        Parse parse,
        Update update
    ) {
        if (const auto result = drogular::ActionAuthSupport::requireAuthentication(context)) {
            return *result;
        }
        if (const auto result = drogular::ActionAuthSupport::requireSessionValue(
                context, "role", "admin", "/dashboard?error=access_denied")
        ) {
            return *result;
        }

        const auto rawId = context.requireRouteParam("id");
        int id = 0;
        const auto [end, error] =
            std::from_chars(rawId.data(), rawId.data() + rawId.size(), id);
        if (error != std::errc{} || end != rawId.data() + rawId.size() || id <= 0) {
            return invalid(context);
        }
        const auto raw = context.form<std::string>(formName);
        const auto value = raw ? parse(*raw) : std::optional<Value>{};
        if (!value.has_value()) {
            return invalid(context);
        }
        if (!update(*context.requireService<PortalTaskProvider>(), id, *value)) {
            return invalid(context);
        }
        if (context.isInteraction()) {
            return render(context, false);
        }
        return drogular::ActionResult::redirect("/dashboard");
    }

private:
    class TasksComponent final : public drogular::TemplateComponent {
    public:
        std::string templatePath() const override {
            return "fragments/dashboard_tasks.html";
        }
    };

    static drogular::ActionResult invalid(drogular::ActionContext& context) {
        if (context.isInteraction()) {
            return render(context, true, drogon::k422UnprocessableEntity);
        }
        return drogular::ActionResult::redirect("/dashboard?error=invalid_task_update");
    }

    static drogular::ActionResult render(
        drogular::ActionContext& context,
        bool invalidUpdate,
        drogon::HttpStatusCode status = drogon::k200OK
    ) {
        return drogular::ActionRenderer::render<TasksComponent>(
            context,
            [&](drogular::RenderContext& renderContext) {
                PortalPageSupport::apply(renderContext, "dashboard.title");
                renderContext.set("taskUpdateError", invalidUpdate);
                PortalDashboardTasksSupport::apply(renderContext);
            },
            status
        );
    }
};

class PortalUpdateTaskStatusAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        return PortalDashboardTaskActionSupport::handle<PortalTaskStatus>(
            context, "status", parsePortalTaskStatus,
            [](PortalTaskProvider& provider, int id, PortalTaskStatus value) {
                return provider.updateStatus(id, value);
            }
        );
    }
};

class PortalUpdateTaskPriorityAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        return PortalDashboardTaskActionSupport::handle<PortalTaskPriority>(
            context, "priority", parsePortalTaskPriority,
            [](PortalTaskProvider& provider, int id, PortalTaskPriority value) {
                return provider.updatePriority(id, value);
            }
        );
    }
};