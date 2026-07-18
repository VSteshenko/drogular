#pragma once

#include "features/projects/providers/project_provider.hpp"
#include "providers/project_type_provider.hpp"
#include "ui/portal_page_support.hpp"
#include "localization/portal_error_translator.hpp"

#include <drogular/page.hpp>
#include <drogular/page_auth_support.hpp>

#include <unordered_map>
#include <algorithm>
#include <string>

class PortalProjectTypesPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        PortalPageSupport::apply(
            context,
            "project_types.page.title"
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

        const auto request =
            context.request();

        const auto error =
            request != nullptr
                ? request->getParameter("error")
                : std::string("");

        const auto success =
            request != nullptr
                ? request->getParameter("success")
                : std::string("");

        const auto createTitle =
            request != nullptr
                ? request->getParameter("title")
                : std::string("");

        const auto projectTypesError =
            PortalErrorTranslator::projectTypesError(
                context,
                error
            );

        const auto projectTypesSuccess =
            PortalErrorTranslator::projectTypesSuccess(
                context,
                success
            );

        context.set("hasProjectTypesError", !projectTypesError.empty());
        context.set("hasProjectTypesSuccess", !projectTypesSuccess.empty());
        context.set("alertMessage", !projectTypesError.empty()
            ? projectTypesError
            : projectTypesSuccess);

        const auto code =
            request != nullptr
                ? request->getParameter("code")
                : std::string("");

        context.set(
            "createProjectTypeCode",
            code
        );

        context.set(
            "createProjectTypeTitle",
            createTitle
        );

        const auto schema =
            PortalSchema::projectTypes();

        context.set(
            "projectTypeCodeLabel",
            context.translate(
                schema.fieldLabelKey("code")
            )
        );

        context.set(
            "projectTypeTitleLabel",
            context.translate(
                schema.fieldLabelKey("title")
            )
        );

        context.set(
            "projectTypeCodeRequired",
            schema.fieldRequired("code")
        );

        context.set(
            "projectTypeTitleRequired",
            schema.fieldRequired("title")
        );

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

            item["title"] =
                type.title;

            item["subtitle"] =
                type.code;

            item["count"] =
                projectCount;

            item["countLabel"] =
                context.translate(
                    "project_types.projects"
                );

            item["canDelete"] =
                projectCount == 0;

            item["editUrl"] =
                "/project-types/" +
                std::to_string(type.id) +
                "/edit";

            item["deleteUrl"] =
                "/project-types/" +
                std::to_string(type.id) +
                "/delete";

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