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

        const auto isAdmin =
            context.get<bool>("isAdmin").value_or(false);

        Json::Value sections(Json::arrayValue);

        Json::Value workspace(Json::objectValue);
        workspace["title"] = context.translate("dashboard.section.workspace");
        workspace["visible"] = true;
        workspace["links"] = Json::Value(Json::arrayValue);

        const auto addLink = [](
            Json::Value& links,
            std::string title,
            std::string url,
            bool visible = true
        ) {
            Json::Value link(Json::objectValue);
            link["title"] = std::move(title);
            link["url"] = std::move(url);
            link["visible"] = visible;
            links.append(std::move(link));
        };

        addLink(
            workspace["links"],
            context.translate("nav.projects"),
            "/projects");
        addLink(
            workspace["links"],
            context.translate("nav.departments"),
            "/departments");
        addLink(
            workspace["links"],
            context.translate("nav.users"),
            "/users");
        sections.append(std::move(workspace));

        Json::Value administration(Json::objectValue);
        administration["title"] = context.translate("dashboard.section.administration");
        administration["visible"] = isAdmin;
        administration["links"] = Json::Value(Json::arrayValue);
        addLink(
            administration["links"],
            context.translate("nav.admin"),
            "/admin",
            isAdmin);
        addLink(
            administration["links"],
            context.translate("roles.manage"),
            "/roles",
            isAdmin);
        addLink(
            administration["links"],
            context.translate("project_types.manage"),
            "/project-types",
            isAdmin);
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