#pragma once

#include <json/json.h>

#include <string>

struct PortalSelectOption {
    std::string value;
    std::string label;
    bool selected{false};
    bool disabled{false};
};

inline Json::Value toJson(const PortalSelectOption& option) {
    Json::Value json(Json::objectValue);
    json["value"] = option.value;
    json["label"] = option.label;
    json["selected"] = option.selected;
    json["disabled"] = option.disabled;
    return json;
}