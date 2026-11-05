#pragma once

#include "ui/portal_page_support.hpp"
#include "ui/portal_frame_navigation_support.hpp"
#include "features/projects/providers/project_provider.hpp"
#include "features/dashboard/ui/portal_dashboard_tasks_support.hpp"
#include "features/departments/providers/department_provider.hpp"
#include "features/users/providers/user_provider.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <algorithm>
#include <string>

class PortalDashboardPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "dashboard.title"
        );

        if (!drogular::PageAuthSupport::requireAuthentication(context)) {
            return;
        }

        applyOverview(context);
        context.set("taskUpdateError", false);
        PortalDashboardTasksSupport::apply(context);

        Json::Value sections(Json::arrayValue);

        const auto addLink = [](
            Json::Value& links,
            std::string titleKey,
            std::string url,
            bool adminOnly = false,
            bool offlineNavigation = false
        ) {
            Json::Value link(Json::objectValue);
            link["titleKey"] = std::move(titleKey);
            link["url"] = url;
            link["fragmentUrl"] =
                PortalFrameNavigationSupport::fragmentUrl(url);
            link["adminOnly"] = adminOnly;
            link["offlineNavigation"] = offlineNavigation;
            links.append(std::move(link));
        };

        Json::Value workspace(Json::objectValue);
        workspace["titleKey"] = "dashboard.section.workspace";
        workspace["adminOnly"] = false;
        workspace["links"] = Json::Value(Json::arrayValue);
        addLink(
            workspace["links"],
            "nav.projects",
            "/projects",
            false,
            true
        );
        addLink(
            workspace["links"],
            "nav.departments",
            "/departments",
            false,
            true
        );
        addLink(
            workspace["links"],
            "nav.users",
            "/users",
            false,
            true
        );
        sections.append(std::move(workspace));

        Json::Value administration(Json::objectValue);
        administration["titleKey"] = "dashboard.section.administration";
        administration["adminOnly"] = true;
        administration["links"] = Json::Value(Json::arrayValue);
        addLink(
            administration["links"],
            "nav.admin",
            "/admin",
            true
        );
        addLink(
            administration["links"],
            "roles.manage",
            "/roles",
            true
        );
        addLink(
            administration["links"],
            "project_types.manage",
            "/project-types",
            true
        );
        sections.append(std::move(administration));

        context.set("dashboardSections", sections);
    }

private:
    static void applyOverview(
        drogular::RenderContext& context
    ) {
        const auto projects =
            context.requireService<PortalProjectProvider>()->all();
        const auto departments =
            context.requireService<PortalDepartmentProvider>()->all();
        const auto users =
            context.requireService<PortalUserProvider>()->all();

        const auto countStatus = [&projects](
            const std::string& status
        ) {
            return static_cast<Json::UInt64>(
                std::count_if(
                    projects.begin(),
                    projects.end(),
                    [&status](const PortalProject& project) {
                        return project.status == status;
                    }
                )
            );
        };

        Json::Value overview(Json::arrayValue);
        const auto addMetric = [&overview](
            const std::string& labelKey,
            Json::UInt64 value,
            const std::string& url
        ) {
            Json::Value metric(Json::objectValue);
            metric["labelKey"] = labelKey;
            metric["value"] = value;
            metric["url"] = url;
            metric["fragmentUrl"] =
                PortalFrameNavigationSupport::fragmentUrl(url);
            overview.append(std::move(metric));
        };

        addMetric(
            "dashboard.metric.projects",
            static_cast<Json::UInt64>(projects.size()),
            "/projects"
        );
        addMetric(
            "dashboard.metric.active",
            countStatus("active"),
            "/projects?status=active"
        );
        addMetric(
            "dashboard.metric.paused",
            countStatus("paused"),
            "/projects?status=paused"
        );
        addMetric(
            "dashboard.metric.departments",
            static_cast<Json::UInt64>(departments.size()),
            "/departments"
        );
        addMetric(
            "dashboard.metric.users",
            static_cast<Json::UInt64>(users.size()),
            "/users"
        );

        context.set("dashboardOverview", overview);
    }

public:
    std::string templatePath() const override {
        return "dashboard.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};