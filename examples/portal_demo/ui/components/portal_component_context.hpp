#pragma once

#include <drogular/component.hpp>
#include <drogular/render_context.hpp>

#include <json/json.h>

#include <sstream>
#include <string>

namespace portal::ui {

inline bool inputFlag(
    const drogular::Component& component,
    const std::string& name
) {
    const auto value = component.input<std::string>(name).value_or("");
    return value == "true" || value == "1" || value == "yes";
}

inline Json::Value resolveJson(
    const drogular::RenderContext& context,
    const std::string& path
) {
    std::istringstream stream(path);
    std::string segment;
    Json::Value value;
    bool first = true;

    while (std::getline(stream, segment, '.')) {
        if (first) {
            value = context.getOr<Json::Value>(segment, Json::Value());
            first = false;
            continue;
        }

        if (!value.isObject() || !value.isMember(segment)) {
            return Json::Value();
        }

        value = value[segment];
    }

    return value;
}

} // namespace portal::ui