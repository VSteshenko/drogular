#pragma once

#include <drogular/developer_tools/application_inspection.hpp>

#include <json/json.h>

#include <utility>

class SystemInfoContributor final
    : public drogular::DeveloperToolsContributor {
public:
    void contribute(
        drogular::ApplicationInspection& inspection
    ) const override {
        Json::Value data(Json::objectValue);
        data["example"] = "developer_tools";
        data["profile"] = "development";
        data["message"] = "Rendered by a component registered independently from inspection data.";

        inspection.addSection({
            .id = "system-info",
            .title = "System Info",
            .component = "example.system-info",
            .data = std::move(data)
        });
    }
};