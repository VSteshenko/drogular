#pragma once

#include "features/localization/support/portal_error_translator.hpp"
#include "features/project_types/providers/project_type_provider.hpp"
#include "features/projects/providers/project_provider.hpp"

#include <drogular/render_context.hpp>

#include <string>
#include <unordered_map>

class PortalProjectTypesListSupport final {
public:
    static void apply(
        drogular::RenderContext& context,
        const std::string& error = {},
        const std::string& success = {}
    ) {
        const auto projectTypesError =
            PortalErrorTranslator::projectTypesError(context, error);
        const auto projectTypesSuccess =
            PortalErrorTranslator::projectTypesSuccess(context, success);

        context.set(
            "hasProjectTypesListError",
            !projectTypesError.empty()
        );
        context.set(
            "hasProjectTypesListSuccess",
            !projectTypesSuccess.empty()
        );
        context.set(
            "projectTypesListAlertMessage",
            !projectTypesError.empty()
                ? projectTypesError
                : projectTypesSuccess
        );

        auto projectTypes =
            context.requireService<PortalProjectTypeProvider>();
        auto projects =
            context.requireService<PortalProjectProvider>();

        std::unordered_map<int, int> projectCounts;
        for (const auto& project : projects->all()) {
            ++projectCounts[project.projectTypeId];
        }

        Json::Value items(Json::arrayValue);
        for (const auto& type : projectTypes->all()) {
            const auto found = projectCounts.find(type.id);
            const auto projectCount =
                found != projectCounts.end()
                    ? found->second
                    : 0;

            Json::Value item(Json::objectValue);
            item["title"] = type.title;
            item["subtitle"] = type.code;
            item["count"] = projectCount;
            item["countLabel"] =
                context.translate("project_types.projects");
            item["canDelete"] = projectCount == 0;
            item["editUrl"] =
                "/project-types/" +
                std::to_string(type.id) +
                "/edit";
            item["deleteUrl"] =
                "/project-types/" +
                std::to_string(type.id) +
                "/delete";
            items.append(std::move(item));
        }

        context.set("projectTypes", items);
    }
};