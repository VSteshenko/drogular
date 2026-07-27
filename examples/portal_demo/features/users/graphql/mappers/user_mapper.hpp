#pragma once

#include "features/users/data/portal_user.hpp"
#include "features/users/data/portal_user_query.hpp"
#include "data/portal_schema.hpp"
#include "data/portal_schema_mapper.hpp"

#include <drogular/graphql_variables.hpp>
#include <drogular/paged_result.hpp>

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

    static drogular::GraphQLVariables toVariables(
        const PortalUserQuery& query
    ) {
        drogular::GraphQLVariables variables;

        if (query.search.has_value()) {
            variables.set("search", *query.search);
        }
        if (query.role.has_value()) {
            variables.set("role", *query.role);
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

                sorting.append(std::move(value));
            }

            variables.set(
                "sorting",
                std::move(sorting)
            );
        }

        return variables;
    }

    static drogular::PagedResult<PortalUser> pageFromValue(
        const Json::Value& value
    ) {
        drogular::PagedResult<PortalUser> page;
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