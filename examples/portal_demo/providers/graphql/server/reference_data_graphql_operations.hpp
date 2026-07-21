#pragma once

#include "data/portal_dataset.hpp"
#include "data/portal_schema.hpp"
#include "data/portal_schema_mapper.hpp"
#include "providers/graphql/server/portal_graphql_operation_registry.hpp"
#include "providers/graphql/server/portal_graphql_operation_support.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class ReferenceDataGraphQLOperations {
public:
    explicit ReferenceDataGraphQLOperations(std::shared_ptr<PortalDataset> dataset)
        : dataset_(std::move(dataset))
    {
    }

    void registerWith(PortalGraphQLOperationRegistry& registry) {
        // Queries
        registry.registerQuery(
            "PortalRoles",
            [this](const auto&) {
                return rolesResponse();
            });
        registry.registerQuery(
            "PortalRoleByCode",
            [this](const auto& variables) {
                return roleByCodeResponse(variables.json()["code"].asString());
            });
        registry.registerQuery(
            "PortalRoleById",
            [this](const auto& variables) {
                return roleByIdResponse(variables.json()["id"].asInt());
            });
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

        // Mutations
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
        registry.registerMutation(
            "CreatePortalRole",
            [this](const auto& variables) {
                return createRoleResponse(variables.json()["role"]);
            });
        registry.registerMutation(
            "UpdatePortalRole",
            [this](const auto& variables) {
                return updateRoleResponse(variables.json()["role"]);
            });
        registry.registerMutation(
            "RemovePortalRole",
            [this](const auto& variables) {
                return removeRoleResponse(variables.json()["id"].asInt());
            });
    }

private:
    drogular::GraphQLResponse rolesResponse() const {
        Json::Value roles(Json::arrayValue);

        for (const auto& role : dataset_->roles()) {
            roles.append(roleJson(role));
        }

        Json::Value data(Json::objectValue);
        data["roles"] = roles;

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse roleByCodeResponse(
        const std::string& code
    ) const {
        Json::Value data(Json::objectValue);
        data["roleByCode"] = Json::Value();

        for (const auto& role : dataset_->roles()) {
            if (role.code == code) {
                data["roleByCode"] =
                    roleJson(role);
                break;
            }
        }

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse roleByIdResponse(
        int id
    ) const {
        Json::Value data(Json::objectValue);
        data["role"] = Json::Value();

        const auto role =
            std::find_if(
                dataset_->roles().begin(),
                dataset_->roles().end(),
                [id](const PortalRole& item) {
                    return item.id == id;
                }
            );

        if (role != dataset_->roles().end()) {
            data["role"] =
                roleJson(*role);
        }

        return PortalGraphQLOperationSupport::response(data);
    }

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

    drogular::GraphQLResponse projectTypeResponse(
        int id
    ) const {
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

    static Json::Value roleJson(
        const PortalRole& role
    ) {
        return PortalSchemaMapper::toJson(
            PortalSchema::roles(),
            role
        );
    }

    int nextProjectTypeId() const {
        int nextId = 1;

        for (const auto& type : dataset_->projectTypes()) {
            nextId =
                std::max(nextId, type.id + 1);
        }

        return nextId;
    }

    drogular::GraphQLResponse createProjectTypeResponse(
        const Json::Value& value
    ) {
        auto type =
            PortalSchemaMapper::fromJson(
                PortalSchema::projectTypes(),
                value
            );

        type.id =
            nextProjectTypeId();

        dataset_->projectTypes().push_back(type);

        Json::Value data(Json::objectValue);
        data["createProjectType"] =
            projectTypeJson(type);

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse updateProjectTypeResponse(
        const Json::Value& value
    ) {
        Json::Value data(Json::objectValue);
        data["updateProjectType"] = Json::Value();

        const auto id =
            value["id"].asInt();

        for (auto& type : dataset_->projectTypes()) {
            if (type.id != id) {
                continue;
            }

            if (value.isMember("code")) {
                type.code =
                    value["code"].asString();
            }

            if (value.isMember("title")) {
                type.title =
                    value["title"].asString();
            }

            data["updateProjectType"] =
                projectTypeJson(type);

            break;
        }

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse removeProjectTypeResponse(
        int id
    ) {
        Json::Value data(Json::objectValue);

        const auto used =
            std::any_of(
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

        auto& types =
            dataset_->projectTypes();

        const auto originalSize =
            types.size();

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

        data["removeProjectType"] =
            types.size() != originalSize;

        return PortalGraphQLOperationSupport::response(data);
    }

    int nextRoleId() const {
        int nextId = 1;

        for (const auto& role : dataset_->roles()) {
            nextId =
                std::max(nextId, role.id + 1);
        }

        return nextId;
    }

    drogular::GraphQLResponse createRoleResponse(
        const Json::Value& value
    ) {
        auto role =
            PortalSchemaMapper::fromJson(
                PortalSchema::roles(),
                value
            );

        role.id =
            nextRoleId();

        dataset_->roles().push_back(role);

        Json::Value data(Json::objectValue);
        data["createRole"] =
            roleJson(role);

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse updateRoleResponse(
        const Json::Value& value
    ) {
        Json::Value data(Json::objectValue);
        data["updateRole"] = Json::Value();

        const auto id =
            value["id"].asInt();

        for (auto& role : dataset_->roles()) {
            if (role.id != id) {
                continue;
            }

            if (value.isMember("code")) {
                role.code =
                    value["code"].asString();
            }

            if (value.isMember("title")) {
                role.title =
                    value["title"].asString();
            }

            data["updateRole"] =
                roleJson(role);

            break;
        }

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse removeRoleResponse(
        int id
    ) {
        Json::Value data(Json::objectValue);

        const auto role =
            std::find_if(
                dataset_->roles().begin(),
                dataset_->roles().end(),
                [id](const PortalRole& item) {
                    return item.id == id;
                }
            );

        if (role == dataset_->roles().end()) {
            data["removeRole"] = false;
            return PortalGraphQLOperationSupport::response(data);
        }

        const auto used =
            std::any_of(
                dataset_->users().begin(),
                dataset_->users().end(),
                [&role](const PortalUser& user) {
                    return user.role == role->code;
                }
            );

        if (used) {
            data["removeRole"] = false;
            return PortalGraphQLOperationSupport::response(data);
        }

        dataset_->roles().erase(role);

        data["removeRole"] = true;

        return PortalGraphQLOperationSupport::response(data);
    }

    std::shared_ptr<PortalDataset> dataset_;
};