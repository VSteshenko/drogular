#pragma once

#include "../../../portal_user.hpp"

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
        Json::Value value(Json::objectValue);

        value["username"] = user.username;
        value["password"] = user.password;
        value["role"] = user.role;

        drogular::GraphQLVariables variables;
        variables.set("user", value);

        return variables;
    }

    static PortalUser fromValue(
        const Json::Value& value
    ) {
        PortalUser user;

        user.username = value["username"].asString();
        user.password = value["password"].asString();
        user.role = value["role"].asString();

        return user;
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