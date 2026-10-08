#pragma once

#include "ui/portal_page_support.hpp"
#include "ui/portal_frame_navigation_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

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

    std::string templatePath() const override {
        return "dashboard.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};