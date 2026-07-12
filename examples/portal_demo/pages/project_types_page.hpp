#pragma once

#include "../providers/project_provider.hpp"
#include "../providers/project_type_provider.hpp"
#include "../ui/portal_page_support.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <unordered_map>

class PortalProjectTypesPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "project_types.title"
        );

        context.set("hasAdminAccess", false);

        if (!drogular::PageAuthSupport::requireAuthentication(
                context
            )) {
            return;
        }

        if (!drogular::PageAuthSupport::requireSessionValue(
                context,
                "role",
                "admin"
            )) {
            return;
        }

        context.set("hasAdminAccess", true);

        auto projectTypes =
            context.requireService<
                PortalProjectTypeProvider
            >();

        auto projects =
            context.requireService<
                PortalProjectProvider
            >();

        std::unordered_map<int, int> projectCounts;

        for (const auto& project : projects->all()) {
            ++projectCounts[project.projectTypeId];
        }

        Json::Value items(Json::arrayValue);

        for (const auto& type : projectTypes->all()) {
            const auto found =
                projectCounts.find(type.id);

            const auto projectCount =
                found != projectCounts.end()
                    ? found->second
                    : 0;

            Json::Value item(Json::objectValue);

            item["id"] = type.id;
            item["code"] = type.code;
            item["title"] = type.title;
            item["projectCount"] = projectCount;
            item["canDelete"] = projectCount == 0;

            items.append(
                std::move(item)
            );
        }

        context.set(
            "projectTypes",
            items
        );
    }

    std::string templatePath() const override {
        return "project_types.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};