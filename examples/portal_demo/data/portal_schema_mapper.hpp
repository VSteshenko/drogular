#pragma once

#include "portal_table_schema.hpp"

#include <json/value.h>

#include <string>
#include <variant>

class PortalSchemaMapper {
public:
    template <typename TModel>
    static Json::Value toJson(
        const PortalTableSchema<TModel>& schema,
        const TModel& model
    ) {
        Json::Value value(Json::objectValue);

        for (const auto& field : schema.fields()) {
            const auto fieldValue =
                field.value(model);

            if (std::holds_alternative<int>(fieldValue)) {
                value[field.name] =
                    std::get<int>(fieldValue);
            } else if (std::holds_alternative<std::string>(fieldValue)) {
                value[field.name] =
                    std::get<std::string>(fieldValue);
            }
        }

        return value;
    }

    template <typename TModel>
    static TModel fromJson(
        const PortalTableSchema<TModel>& schema,
        const Json::Value& json
    ) {
        TModel model;

        for (const auto& field : schema.fields()) {
            if (!json.isMember(field.name)) {
                continue;
            }

            const auto& value =
                json[field.name];

            if (value.isInt()) {
                field.setValue(
                    model,
                    value.asInt()
                );
            } else if (value.isString()) {
                field.setValue(
                    model,
                    value.asString()
                );
            }
        }

        return model;
    }
};