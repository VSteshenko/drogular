#pragma once

#include "data/portal_dataset.hpp"
#include "data/portal_schema.hpp"
#include "data/portal_schema_mapper.hpp"
#include "core/graphql/server/portal_graphql_operation_support.hpp"

#include <drogular/graphql_operation_registry.hpp>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

class DepartmentGraphQLOperations {
public:
    explicit DepartmentGraphQLOperations(std::shared_ptr<PortalDataset> dataset)
        : dataset_(std::move(dataset))
    {
    }

    void registerWith(drogular::GraphQLOperationRegistry& registry) {
        // Queries
        registry.registerQuery(
            "PortalDepartments",
            [this](const auto&) {
                return departmentsResponse();
            });
        registry.registerQuery(
            "PortalDepartmentById",
            [this](const auto& variables) {
                return departmentResponse(variables.json()["id"].asInt());
            });
        registry.registerQuery(
            "SearchPortalDepartments",
            [this](const auto& variables) {
                return searchDepartmentsResponse(variables.json());
            });
        // Mutations
        registry.registerMutation(
            "CreatePortalDepartment",
            [this](const auto& variables) {
                return createDepartmentResponse(variables.json()["department"]);
            });
        registry.registerMutation(
            "UpdatePortalDepartment",
            [this](const auto& variables) {
                return updateDepartmentResponse(variables.json()["department"]);
            });
    }

private:
    static Json::Value departmentJson(
        const PortalDepartment& value
    ) {
        return PortalSchemaMapper::toJson(PortalSchema::departments(), value);
    }

    int nextDepartmentId() const {
        int id = 1;
        for (const auto& department: dataset_->departments()) {
            id = std::max(id,department.id + 1);
        }

        return id;
    }

    drogular::GraphQLResponse departmentsResponse() const {
        Json::Value departments(Json::arrayValue);
        for (const auto& department: dataset_->departments()) {
            departments.append(departmentJson(department));
        }

        Json::Value data(Json::objectValue);
        data["departments"] = departments;

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse departmentResponse(
        int id
    ) const {
        Json::Value data(Json::objectValue);

        data["department"] = Json::Value();
        for (const auto& department:dataset_->departments()) {
            if (department.id == id) {
                data["department"] = departmentJson(department);
                break;
            }
        }

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse searchDepartmentsResponse(
        const Json::Value& value
    ) const {
        std::vector<PortalDepartment> values;
        const auto needle=
            PortalGraphQLOperationSupport::lowercase(
                value.get("search",""
            ).asString());
        for (const auto& department:dataset_->departments()) {
            if (!needle.empty() &&
                PortalGraphQLOperationSupport::lowercase(
                    department.name + " " + department.description
                ).find(needle) == std::string::npos
            ) {
                continue;
            }
            if(value.isMember("isActive") &&
                department.isActive!=value["isActive"].asBool()
            ) {
                continue;
            }

            values.push_back(department);
        }
        std::stable_sort(
            values.begin(),
            values.end(),
            [](const auto& left,const auto& right) {
                return left.name < right.name;
            }
        );

        int page = std::max(1,value.get("page",1).asInt());
        int size = std::max(1,value.get("pageSize",10).asInt());
        int total = values.size();
        int pages = std::max(1,(total + size - 1) / size);
        size_t begin = (page - 1) * size;
        size_t end = std::min(values.size(),begin + size_t(size));

        Json::Value items(Json::arrayValue);
        if (begin < values.size()) {
            for (size_t i = begin; i < end; ++i) {
                items.append(departmentJson(values[i]));
            }
        }

        Json::Value departmentPage(Json::objectValue);

        departmentPage["items"] = items;
        departmentPage["page"]= page;
        departmentPage["pageSize"]= size;
        departmentPage["totalItems"]=total;
        departmentPage["totalPages"]=pages;

        Json::Value data(Json::objectValue);
        data["departmentPage"] = departmentPage;

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse createDepartmentResponse(
        const Json::Value& value
    ) {
        auto department =
            PortalSchemaMapper::fromJson(PortalSchema::departments(), value);
        department.id = nextDepartmentId();
        dataset_->departments().push_back(department);

        Json::Value data(Json::objectValue);
        data["createDepartment"] = departmentJson(department);

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse updateDepartmentResponse(
        const Json::Value& value
    ) {
        Json::Value department(Json::objectValue);

        department["updateDepartment"] = Json::Value();
        for (auto& item: dataset_->departments()) {
            if (item.id == value["id"].asInt()) {
                if (value.isMember("name")) {
                    item.name = value["name"].asString();
                }
                if (value.isMember("description")) {
                    item.description = value["description"].asString();
                }
                if (value.isMember("managerId")) {
                    item.managerId = value["managerId"].asInt();
                }
                if (value.isMember("isActive")) {
                    item.isActive = value["isActive"].asBool();
                }
                department["updateDepartment"] = departmentJson(item);

                break;
            }
        }

        return PortalGraphQLOperationSupport::response(department);
    }

    std::shared_ptr<PortalDataset> dataset_;
};