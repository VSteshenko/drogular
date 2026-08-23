#pragma once

#include "ui/portal_page_support.hpp"

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
            bool adminOnly = false
        ) {
            Json::Value link(Json::objectValue);
            link["titleKey"] = std::move(titleKey);
            link["url"] = std::move(url);
            link["adminOnly"] = adminOnly;
            links.append(std::move(link));
        };

        Json::Value workspace(Json::objectValue);
        workspace["titleKey"] = "dashboard.section.workspace";
        workspace["adminOnly"] = false;
        workspace["links"] = Json::Value(Json::arrayValue);
        addLink(
            workspace["links"],
            "nav.projects",
            "/projects"
        );
        addLink(
            workspace["links"],
            "nav.departments",
            "/departments"
        );
        addLink(
            workspace["links"],
            "nav.users",
            "/users"
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