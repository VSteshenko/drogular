#pragma once

#include "features/department_members/data/portal_department_member.hpp"

#include <drogular/graphql_variables.hpp>

#include <json/value.h>
#include <optional>
#include <vector>

class DepartmentMemberMapper {
public:
    static PortalDepartmentMember fromValue(
        const Json::Value& value
    ) {
        return {
            value.get("id",0).asInt(),
            value.get("departmentId",0).asInt(),
            value.get("userId",0).asInt()
        };
    }

    static std::vector<PortalDepartmentMember> fromList(
        const Json::Value& values
    ) {
        std::vector<PortalDepartmentMember> result;

        for (const auto& value: values) {
            result.push_back(fromValue(value));
        }

        return result;
    }

    static std::optional<PortalDepartmentMember> optionalMember(
        const std::optional<Json::Value>& value
    ) {
        if (!value || value->isNull()) {
            return std::nullopt;
        }
        return fromValue(*value);
    }

    static drogular::GraphQLVariables variables(
        int departmentId,
        int userId
    ) {
        drogular::GraphQLVariables result;

        result.set("departmentId", departmentId);
        result.set("userId", userId);

        return result;
    }

    static drogular::GraphQLVariables departmentVariables(
        int id
    ) {
        drogular::GraphQLVariables result;

        result.set("departmentId", id);

        return result;
    }

    static drogular::GraphQLVariables userVariables(
        int id
    ) {
        drogular::GraphQLVariables result;

        result.set("userId", id);

        return result;
    }
};