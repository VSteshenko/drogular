#pragma once

#include "../../data/portal_dataset.hpp"
#include "../../portal_project.hpp"

#include <drogular/static_graphql_client.hpp>

#include <json/value.h>
#include <memory>

class PortalDatasetGraphQLAdapter {
public:
    explicit PortalDatasetGraphQLAdapter(
        const PortalDataset& dataset
    )
        : dataset_(dataset)
    {
    }

    std::shared_ptr<drogular::StaticGraphQLClient>
    projectClient() const {
        return std::make_shared<drogular::StaticGraphQLClient>(
            projectGraphQLData()
        );
    }

private:
    Json::Value projectGraphQLData() const {
        Json::Value projects(Json::arrayValue);

        for (const auto& project : dataset_.projects()) {
            projects.append(
                projectJson(project)
            );
        }

        Json::Value data(Json::objectValue);

        data["projects"] = projects;

        if (!dataset_.projects().empty()) {
            data["project"] =
                projectJson(dataset_.projects().front());
        } else {
            data["project"] =
                Json::Value();
        }

        return data;
    }

    static Json::Value projectJson(
        const PortalProject& project
    ) {
        Json::Value value(Json::objectValue);

        value["id"] = project.id;
        value["title"] = project.title;
        value["status"] = project.status;

        return value;
    }

    const PortalDataset& dataset_;
};