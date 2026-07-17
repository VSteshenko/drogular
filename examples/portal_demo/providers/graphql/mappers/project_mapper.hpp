#pragma once

#include "../../../data/models/portal_project.hpp"
#include "../../../data/models/portal_project_update.hpp"
#include "../../../data/models/portal_project_query.hpp"
#include "../../../data/models/portal_page.hpp"
#include "../../../data/portal_schema.hpp"
#include "../../../data/portal_schema_mapper.hpp"

#include <drogular/graphql_variables.hpp>
#include <drogular/graphql_response.hpp>

#include <json/value.h>
#include <vector>

class ProjectMapper {
public:
    static drogular::GraphQLVariables toVariables(
        const PortalProject& project
    ) {
        const auto value =
            PortalSchemaMapper::toJson(
                PortalSchema::projects(),
                project
            );

        drogular::GraphQLVariables variables;

        variables.set(
            "project",
            value
        );

        return variables;
    }

    static drogular::GraphQLVariables toVariables(
        const PortalProjectUpdate& input
    ) {
        Json::Value project(Json::objectValue);

        project["id"] = input.id;

        if (input.title.has_value()) {
            project["title"] = *input.title;
        }

        if (input.status.has_value()) {
            project["status"] = *input.status;
        }

        if (input.projectTypeId.has_value()) {
            project["projectTypeId"] =
                *input.projectTypeId;
        }

        drogular::GraphQLVariables variables;

        variables.set(
            "project",
            std::move(project)
        );

        return variables;
    }

    static drogular::GraphQLVariables toVariables(
        const PortalProjectQuery& query
    ) {
        drogular::GraphQLVariables variables;

        if (query.search.has_value()) {
            variables.set(
                "search",
                *query.search
            );
        }

        if (query.status.has_value()) {
            variables.set(
                "status",
                *query.status
            );
        }

        if (query.projectTypeId.has_value()) {
            variables.set(
                "projectTypeId",
                *query.projectTypeId
            );
        }

        if (query.ownerId.has_value()) {
            variables.set(
                "ownerId",
                *query.ownerId
            );
        }

        variables.set("page", query.page);
        variables.set("pageSize", query.pageSize);

        if (!query.sorting.empty()) {
            Json::Value sorting(Json::arrayValue);

            for (const auto& item :
                query.sorting) {
                Json::Value value(Json::objectValue);

                value["field"] = item.field;
                value["direction"] = toString(item.direction);

                sorting.append(
                    std::move(value)
                );
            }

            variables.set(
                "sorting",
                std::move(sorting)
            );
        }

        return variables;
    }

    static PortalPage<PortalProject> pageFromValue(
        const Json::Value& value
    ) {
        PortalPage<PortalProject> page;

        if (value.isNull() || !value.isObject()) {
            return page;
        }

        page.items = fromList(value["items"]);
        page.page = value.get("page", 1).asInt();
        page.pageSize = value.get("pageSize", 10).asInt();
        page.totalItems = value.get("totalItems", 0).asInt();
        page.totalPages = value.get("totalPages", 1).asInt();

        return page;
    }

    static PortalProject fromValue(
        const Json::Value& value
    ) {
        return PortalSchemaMapper::fromJson(
            PortalSchema::projects(),
            value
        );
    }

    static std::vector<PortalProject> fromList(
        const Json::Value& values
    ) {
        std::vector<PortalProject> projects;

        for (const auto& value : values) {
            projects.push_back(
                fromValue(value)
            );
        }

        return projects;
    }

    static drogular::GraphQLVariables idVariables(
        int id
    ) {
        drogular::GraphQLVariables variables;

        variables.set("id", id);

        return variables;
    }

    static std::optional<PortalProject> optionalProject(
        const std::optional<Json::Value>& value
    ) {
        if (!value.has_value() ||
            value->isNull()) {
            return std::nullopt;
        }

        return fromValue(*value);
    }
};