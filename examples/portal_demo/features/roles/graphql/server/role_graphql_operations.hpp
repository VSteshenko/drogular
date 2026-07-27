#pragma once

#include "core/graphql/server/portal_graphql_operation_support.hpp"
#include "data/portal_dataset.hpp"
#include "data/portal_schema.hpp"
#include "data/portal_schema_mapper.hpp"

#include <drogular/graphql_operation_registry.hpp>

#include <algorithm>
#include <memory>
#include <string>

class RoleGraphQLOperations {
public:
    explicit RoleGraphQLOperations(std::shared_ptr<PortalDataset> dataset)
        : dataset_(std::move(dataset))
    {
    }

    void registerWith(drogular::GraphQLOperationRegistry& registry) {
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
    static Json::Value roleJson(const PortalRole& role) {
        return PortalSchemaMapper::toJson(
            PortalSchema::roles(),
            role
        );
    }

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
                data["roleByCode"] = roleJson(role);
                break;
            }
        }

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse roleByIdResponse(int id) const {
        Json::Value data(Json::objectValue);
        data["role"] = Json::Value();

        const auto role = std::find_if(
            dataset_->roles().begin(),
            dataset_->roles().end(),
            [id](const PortalRole& item) {
                return item.id == id;
            }
        );

        if (role != dataset_->roles().end()) {
            data["role"] = roleJson(*role);
        }

        return PortalGraphQLOperationSupport::response(data);
    }

    int nextRoleId() const {
        int nextId = 1;

        for (const auto& role : dataset_->roles()) {
            nextId = std::max(nextId, role.id + 1);
        }

        return nextId;
    }

    drogular::GraphQLResponse createRoleResponse(
        const Json::Value& value
    ) {
        auto role = PortalSchemaMapper::fromJson(
            PortalSchema::roles(),
            value
        );

        role.id = nextRoleId();
        dataset_->roles().push_back(role);

        Json::Value data(Json::objectValue);
        data["createRole"] = roleJson(role);

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse updateRoleResponse(
        const Json::Value& value
    ) {
        Json::Value data(Json::objectValue);
        data["updateRole"] = Json::Value();

        const auto id = value["id"].asInt();

        for (auto& role : dataset_->roles()) {
            if (role.id != id) {
                continue;
            }

            if (value.isMember("code")) {
                role.code = value["code"].asString();
            }

            if (value.isMember("title")) {
                role.title = value["title"].asString();
            }

            data["updateRole"] = roleJson(role);
            break;
        }

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse removeRoleResponse(int id) {
        Json::Value data(Json::objectValue);

        const auto role = std::find_if(
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

        const auto used = std::any_of(
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