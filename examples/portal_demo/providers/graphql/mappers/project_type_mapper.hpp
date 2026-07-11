#pragma once

#include "../../../data/models/portal_project_type.hpp"

#include <drogular/graphql_variables.hpp>

#include <json/value.h>
#include <optional>
#include <vector>

class ProjectTypeMapper {
public:
    static drogular::GraphQLVariables idVariables(
        int id
    ) {
        drogular::GraphQLVariables variables;
        variables.set("id", id);
        return variables;
    }

    static PortalProjectType fromValue(
        const Json::Value& value
    ) {
        return PortalSchemaMapper::fromJson(
            PortalSchema::projectTypes(),
            value
        );
    }

    static std::vector<PortalProjectType> fromList(
        const Json::Value& values
    ) {
        std::vector<PortalProjectType> types;

        for (const auto& value : values) {
            types.push_back(fromValue(value));
        }

        return types;
    }

    static std::optional<PortalProjectType> optionalType(
        const std::optional<Json::Value>& value
    ) {
        if (!value.has_value() || value->isNull()) {
            return std::nullopt;
        }

        return fromValue(*value);
    }
};