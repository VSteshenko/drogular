#pragma once

#include "core/graphql/server/portal_graphql_operation_registry.hpp"
#include "core/graphql/server/portal_graphql_operation_support.hpp"
#include "data/portal_dataset.hpp"
#include "data/portal_schema.hpp"
#include "data/portal_schema_mapper.hpp"

#include <algorithm>
#include <memory>

class ProjectTypeGraphQLOperations {
public:
    explicit ProjectTypeGraphQLOperations(std::shared_ptr<PortalDataset> dataset)
        : dataset_(std::move(dataset))
    {
    }

    void registerWith(PortalGraphQLOperationRegistry& registry) {
        registry.registerQuery(
            "PortalProjectTypes",
            [this](const auto&) {
                return projectTypesResponse();
            });
        registry.registerQuery(
            "PortalProjectTypeById",
            [this](const auto& variables) {
                return projectTypeResponse(variables.json()["id"].asInt());
            });

        registry.registerMutation(
            "CreatePortalProjectType",
            [this](const auto& variables) {
                return createProjectTypeResponse(variables.json()["projectType"]);
            });
        registry.registerMutation(
            "UpdatePortalProjectType",
            [this](const auto& variables) {
                return updateProjectTypeResponse(variables.json()["projectType"]);
            });
        registry.registerMutation(
            "RemovePortalProjectType",
            [this](const auto& variables) {
                return removeProjectTypeResponse(variables.json()["id"].asInt());
            });
    }

private:
    static Json::Value projectTypeJson(
        const PortalProjectType& type
    ) {
        return PortalSchemaMapper::toJson(
            PortalSchema::projectTypes(),
            type
        );
    }

    drogular::GraphQLResponse projectTypesResponse() const {
        Json::Value values(Json::arrayValue);

        for (const auto& type : dataset_->projectTypes()) {
            values.append(projectTypeJson(type));
        }

        Json::Value data(Json::objectValue);
        data["projectTypes"] = values;

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse projectTypeResponse(int id) const {
        Json::Value data(Json::objectValue);
        data["projectType"] = Json::Value();

        for (const auto& type : dataset_->projectTypes()) {
            if (type.id == id) {
                data["projectType"] = projectTypeJson(type);
                break;
            }
        }

        return PortalGraphQLOperationSupport::response(data);
    }

    int nextProjectTypeId() const {
        int nextId = 1;

        for (const auto& type : dataset_->projectTypes()) {
            nextId = std::max(nextId, type.id + 1);
        }

        return nextId;
    }

    drogular::GraphQLResponse createProjectTypeResponse(
        const Json::Value& value
    ) {
        auto type = PortalSchemaMapper::fromJson(
            PortalSchema::projectTypes(),
            value
        );

        type.id = nextProjectTypeId();
        dataset_->projectTypes().push_back(type);

        Json::Value data(Json::objectValue);
        data["createProjectType"] = projectTypeJson(type);

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse updateProjectTypeResponse(
        const Json::Value& value
    ) {
        Json::Value data(Json::objectValue);
        data["updateProjectType"] = Json::Value();

        const auto id = value["id"].asInt();

        for (auto& type : dataset_->projectTypes()) {
            if (type.id != id) {
                continue;
            }

            if (value.isMember("code")) {
                type.code = value["code"].asString();
            }

            if (value.isMember("title")) {
                type.title = value["title"].asString();
            }

            data["updateProjectType"] = projectTypeJson(type);
            break;
        }

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse removeProjectTypeResponse(int id) {
        Json::Value data(Json::objectValue);

        const auto used = std::any_of(
            dataset_->projects().begin(),
            dataset_->projects().end(),
            [id](const PortalProject& project) {
                return project.projectTypeId == id;
            }
        );

        if (used) {
            data["removeProjectType"] = false;
            return PortalGraphQLOperationSupport::response(data);
        }

        auto& types = dataset_->projectTypes();
        const auto originalSize = types.size();

        types.erase(
            std::remove_if(
                types.begin(),
                types.end(),
                [id](const PortalProjectType& type) {
                    return type.id == id;
                }
            ),
            types.end()
        );

        data["removeProjectType"] = types.size() != originalSize;

        return PortalGraphQLOperationSupport::response(data);
    }

    std::shared_ptr<PortalDataset> dataset_;
};
