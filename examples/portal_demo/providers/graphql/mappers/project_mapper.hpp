#pragma once

#include "../../../data/models/portal_project.hpp"
#include "../../../data/models/portal_project_update.hpp"
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