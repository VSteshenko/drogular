#pragma once

#include "features/departments/data/portal_department.hpp"
#include "features/departments/data/portal_department_update.hpp"
#include "features/departments/data/portal_department_query.hpp"
#include "data/models/portal_page.hpp"
#include "data/portal_schema.hpp"
#include "data/portal_schema_mapper.hpp"

#include <drogular/graphql_variables.hpp>

#include <json/value.h>

#include <optional>
#include <vector>

class DepartmentMapper {
public:
    static drogular::GraphQLVariables toVariables(
        const PortalDepartment& department
    ) {
        drogular::GraphQLVariables variables;
        variables.set(
            "department",
            PortalSchemaMapper::toJson(PortalSchema::departments(), department)
        );

        return variables;
    }

    static drogular::GraphQLVariables toVariables(
        const PortalDepartmentUpdate& input
    ) {
        Json::Value value(Json::objectValue);

        value["id"] = input.id;

        if (input.name) {
            value["name"] = *input.name;
        }

        if (input.description) {
            value["description"] = *input.description;
        }

        if (input.managerId) {
            value["managerId"] = *input.managerId;
        }

        if (input.isActive) {
            value["isActive"] = *input.isActive;
        }

        drogular::GraphQLVariables variables;
        variables.set("department", std::move(value));
        return variables;
    }

    static drogular::GraphQLVariables toVariables(
        const PortalDepartmentQuery& query
    ) {
        drogular::GraphQLVariables variables;

        if (query.search) {
            variables.set("search", *query.search);
        }

        if (query.isActive) {
            variables.set("isActive", *query.isActive);
        }

        variables.set("page", query.page);
        variables.set("pageSize", query.pageSize);

        if (!query.sorting.empty()) {
            Json::Value sorting(Json::arrayValue);
            for (const auto& item : query.sorting) {
                Json::Value value(Json::objectValue);

                value["field"] = item.field;
                value["direction"] = toString(item.direction);

                sorting.append(std::move(value));
            }

            variables.set("sorting", std::move(sorting));
        }

        return variables;
    }

    static PortalDepartment fromValue(
        const Json::Value& value
    ) {
        return PortalSchemaMapper::fromJson(PortalSchema::departments(), value);
    }

    static std::vector<PortalDepartment> fromList(
        const Json::Value& values
    ) {
        std::vector<PortalDepartment> result;
        for (const auto& value : values) {
            result.push_back(fromValue(value));
        }
        return result;
    }

    static std::optional<PortalDepartment> optionalDepartment(
        const std::optional<Json::Value>& value
    ) {
        if (!value || value->isNull()) {
            return std::nullopt;
        }
        return fromValue(*value);
    }

    static PortalPage<PortalDepartment> pageFromValue(
        const Json::Value& value
    ) {
        PortalPage<PortalDepartment> page;

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

    static drogular::GraphQLVariables idVariables(
        int id
    ) {
        drogular::GraphQLVariables variables;

        variables.set("id", id);

        return variables;
    }
};