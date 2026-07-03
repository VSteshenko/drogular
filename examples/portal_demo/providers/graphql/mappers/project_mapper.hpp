#pragma once

#include "../../../portal_project.hpp"

#include <drogular/graphql.hpp>
#include <drogular/graphql_variables.hpp>
#include <drogular/graphql_response.hpp>

#include <json/value.h>
#include <vector>

class ProjectMapper {
public:
    static drogular::GraphQLVariables toVariables(
        const PortalProject& project
    ) {
        Json::Value value;

        if (project.id > 0) {
            value["id"] = project.id;
        }

        value["title"] = project.title;
        value["status"] = project.status;

        drogular::GraphQLVariables variables;

        variables.set(
            "project",
            value
        );

        return variables;
    }

    static PortalProject fromValue(
        const Json::Value& value
    ) {
        PortalProject project;

        project.id = value["id"].asInt();
        project.title = value["title"].asString();
        project.status = value["status"].asString();

        return project;
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