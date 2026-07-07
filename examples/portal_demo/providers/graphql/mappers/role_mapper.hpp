#pragma once

#include "../../../portal_role.hpp"
#include "../../../data/portal_schema.hpp"
#include "../../../data/portal_schema_mapper.hpp"

#include <drogular/graphql_variables.hpp>

#include <json/value.h>
#include <string>
#include <vector>

class RoleMapper {
public:
    static drogular::GraphQLVariables rolesVariables(
        const std::string& code
    ) {
        drogular::GraphQLVariables variables;

        variables
            .set("code", code);

        return variables;
    }

    static drogular::GraphQLVariables toVariables(
        const PortalRole& role
    ) {
        const auto value =
            PortalSchemaMapper::toJson(
                PortalSchema::roles(),
                role
            );

        drogular::GraphQLVariables variables;
        variables.set("role", value);

        return variables;
    }

    static PortalRole fromValue(
        const Json::Value& value
    ) {
        return PortalSchemaMapper::fromJson(
            PortalSchema::roles(),
            value
        );
    }

    static std::vector<PortalRole> fromList(
        const Json::Value& values
    ) {
        std::vector<PortalRole> roles;

        for (const auto& value : values) {
            roles.push_back(
                fromValue(value)
            );
        }

        return roles;
    }

    static std::optional<PortalRole> optionalRole(
        const std::optional<Json::Value>& value
    ) {
        if (!value.has_value() ||
            value->isNull()) {
            return std::nullopt;
            }

        return fromValue(*value);
    }
};