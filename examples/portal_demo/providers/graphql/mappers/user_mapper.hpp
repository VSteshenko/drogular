#pragma once

#include "../../../data/models/portal_user.hpp"
#include "../../../data/portal_schema.hpp"
#include "../../../data/portal_schema_mapper.hpp"

#include <drogular/graphql_variables.hpp>

#include <json/value.h>
#include <optional>
#include <string>
#include <vector>

class UserMapper {
public:
    static drogular::GraphQLVariables credentialsVariables(
        const std::string& username,
        const std::string& password
    ) {
        drogular::GraphQLVariables variables;

        variables
            .set("username", username)
            .set("password", password);

        return variables;
    }

    static drogular::GraphQLVariables toVariables(
        const PortalUser& user
    ) {
        const auto value =
            PortalSchemaMapper::toJson(
                PortalSchema::users(),
                user
            );

        drogular::GraphQLVariables variables;
        variables.set("user", value);

        return variables;
    }

    static drogular::GraphQLVariables toVariables(
        const PortalUserUpdate& input
    ) {
        Json::Value user(Json::objectValue);

        user["id"] = input.id;

        if (input.username.has_value()) {
            user["username"] = *input.username;
        }

        if (input.role.has_value()) {
            user["role"] = *input.role;
        }

        drogular::GraphQLVariables variables;

        variables.set(
            "user",
            std::move(user)
        );

        return variables;
    }

    static PortalUser fromValue(
        const Json::Value& value
    ) {
        return PortalSchemaMapper::fromJson(
            PortalSchema::users(),
            value
        );
    }

    static std::vector<PortalUser> fromList(
        const Json::Value& values
    ) {
        std::vector<PortalUser> users;

        for (const auto& value : values) {
            users.push_back(
                fromValue(value)
            );
        }

        return users;
    }

    static std::optional<PortalUser> optionalUser(
        const std::optional<Json::Value>& value
    ) {
        if (!value.has_value() ||
            value->isNull()) {
            return std::nullopt;
        }

        return fromValue(*value);
    }
};