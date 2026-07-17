#pragma once

#include <json/json.h>

#include <string>

namespace portal {
struct PortalProjectFilterViewModel {
    std::string search;

    Json::Value statusOptions{
        Json::arrayValue
    };

    Json::Value projectTypeOptions{
        Json::arrayValue
    };

    Json::Value ownerOptions{
        Json::arrayValue
    };

    Json::Value sortOptions{
        Json::arrayValue
    };

    Json::Value sortDirectionOptions{
        Json::arrayValue
    };

    bool hasActiveFilters = false;
};

inline Json::Value toJson(
    const PortalProjectFilterViewModel& value
) {
    Json::Value json(Json::objectValue);

    json["search"] = value.search;
    json["statusOptions"] = value.statusOptions;
    json["projectTypeOptions"] = value.projectTypeOptions;
    json["ownerOptions"] = value.ownerOptions;
    json["hasActiveFilters"] = value.hasActiveFilters;
    json["sortOptions"] = value.sortOptions;
    json["sortDirectionOptions"] = value.sortDirectionOptions;

    return json;
}

} // namespace portal