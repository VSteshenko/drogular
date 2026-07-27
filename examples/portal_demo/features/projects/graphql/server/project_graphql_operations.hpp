#pragma once

#include "data/portal_dataset.hpp"
#include "data/portal_schema.hpp"
#include "data/portal_schema_mapper.hpp"
#include "features/projects/data/portal_project_sort.hpp"
#include "core/graphql/server/portal_graphql_operation_registry.hpp"
#include "core/graphql/server/portal_graphql_operation_support.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class ProjectGraphQLOperations {
public:
    explicit ProjectGraphQLOperations(std::shared_ptr<PortalDataset> dataset)
        : dataset_(std::move(dataset))
    {
    }

    void registerWith(PortalGraphQLOperationRegistry& registry) {
        // Queries
        registry.registerQuery(
            "PortalProjects",
            [this](const auto&) {
                return projectsResponse();
            });
        registry.registerQuery(
            "PortalProjectById",
            [this](const auto& variables) {
                return projectResponse(variables.json()["id"].asInt());
            });
        registry.registerQuery(
            "SearchPortalProjects",
            [this](const auto& variables) {
                const auto& json = variables.json();
                const auto search =
                    json.isMember("search")
                        ? json["search"].asString()
                        : std::string();
                const auto status =
                    json.isMember("status")
                        ? json["status"].asString()
                        : std::string();
                const auto projectTypeId =
                    json.isMember("projectTypeId")
                        ? std::optional<int>(json["projectTypeId"].asInt())
                        : std::nullopt;
                const auto ownerId =
                    json.isMember("ownerId")
                        ? std::optional<int>(json["ownerId"].asInt())
                        : std::nullopt;

                std::vector<PortalProjectSort> sorting;
                if (json.isMember("sorting") && json["sorting"].isArray()) {
                    for (const auto& item : json["sorting"]) {
                        const auto field = item["field"].asString();
                        if (field != "id" && field != "title" && field != "status") {
                            continue;
                        }

                        sorting.push_back({
                            .field = field,
                            .direction = item["direction"].asString() == "desc"
                                ? PortalSortDirection::Descending
                                : PortalSortDirection::Ascending
                        });
                    }
                }

                const auto page =
                    json.isMember("page")
                        ? std::max(1, json["page"].asInt())
                        : 1;
                const auto pageSize =
                    json.isMember("pageSize")
                        ? std::max(1, json["pageSize"].asInt())
                        : 10;

                return searchProjectsResponse(
                    search,
                    status,
                    projectTypeId,
                    ownerId,
                    sorting,
                    page,
                    pageSize
                );
        });
        // Mutations
        registry.registerMutation(
            "CreatePortalProject",
            [this](const auto& variables) {
                return createProjectResponse(variables.json()["project"]);
            });
        registry.registerMutation(
            "UpdatePortalProject",
            [this](const auto& variables) {
                return updateProjectResponse(variables.json()["project"]);
            });
        registry.registerMutation(
            "RemovePortalProject",
            [this](const auto& variables) {
                return removeProjectResponse(variables.json()["id"].asInt());
            });
    }

private:
    static Json::Value projectJson(
        const PortalProject& project
    ) {
        return PortalSchemaMapper::toJson(
            PortalSchema::projects(),
            project
        );
    }

    drogular::GraphQLResponse projectsResponse() const {
        Json::Value projects(Json::arrayValue);

        for (const auto& project : dataset_->projects()) {
            projects.append(projectJson(project));
        }

        Json::Value data(Json::objectValue);
        data["projects"] = projects;

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse projectResponse(
        int id
    ) const {
        Json::Value data(Json::objectValue);
        data["project"] = Json::Value();

        for (const auto& project : dataset_->projects()) {
            if (project.id == id) {
                data["project"] = projectJson(project);
                break;
            }
        }

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse createProjectResponse(
        const Json::Value& value
    ) {
        auto project =
            PortalSchemaMapper::fromJson(
                PortalSchema::projects(),
                value
            );

        project.id =
            nextProjectId();

        dataset_->projects().push_back(project);

        Json::Value data(Json::objectValue);
        data["createProject"] = projectJson(project);

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse updateProjectResponse(
        const Json::Value& value
    ) {
        Json::Value data(Json::objectValue);
        data["updateProject"] = Json::Value();

        const auto id =
            value["id"].asInt();

        for (auto& project : dataset_->projects()) {
            if (project.id != id) {
                continue;
            }

            if (value.isMember("title")) {
                project.title =
                    value["title"].asString();
            }

            if (value.isMember("status")) {
                project.status =
                    value["status"].asString();
            }

            if (value.isMember("projectTypeId")) {
                project.projectTypeId =
                    value["projectTypeId"].asInt();
            }

            data["updateProject"] =
                projectJson(project);

            break;
        }

        return PortalGraphQLOperationSupport::response(data);
    }

    drogular::GraphQLResponse removeProjectResponse(
        int id
    ) {
        auto& projects =
            dataset_->projects();

        const auto originalSize =
            projects.size();

        projects.erase(
            std::remove_if(
                projects.begin(),
                projects.end(),
                [id](const PortalProject& project) {
                    return project.id == id;
                }
            ),
            projects.end()
        );

        Json::Value data(Json::objectValue);
        data["removeProject"] =
            projects.size() != originalSize;

        return PortalGraphQLOperationSupport::response(data);
    }

    int nextProjectId() const {
        int nextId = 1;

        for (const auto& project : dataset_->projects()) {
            nextId =
                std::max(nextId, project.id + 1);
        }

        return nextId;
    }

    drogular::GraphQLResponse searchProjectsResponse(
        const std::string& search,
        const std::string& status,
        const std::optional<int>& projectTypeId,
        const std::optional<int>& ownerId,
        std::vector<PortalProjectSort> sorting,
        int page,
        int pageSize
    ) const {
        const auto needle =
            PortalGraphQLOperationSupport::lowercase(search);

        std::vector<PortalProject> result;

        for (const auto& project :
            dataset_->projects()) {
            if (!needle.empty() &&
                PortalGraphQLOperationSupport::lowercase(project.title).find(needle) ==
                    std::string::npos) {
                continue;
            }

            if (!status.empty() &&
                project.status != status) {
                continue;
            }

            if (projectTypeId.has_value() &&
                project.projectTypeId !=
                    *projectTypeId) {
                continue;
            }

            if (ownerId.has_value() &&
                project.ownerId != *ownerId) {
                continue;
            }

            result.push_back(project);
        }

        if (sorting.empty()) {
            sorting.push_back({
                .field = "title",
                .direction = PortalSortDirection::Ascending
            });
        }

        const auto compare =
            []<typename T>(
                const T& left,
                const T& right,
                PortalSortDirection direction
            ) {
                return direction == PortalSortDirection::Ascending
                    ? left < right
                    : right < left;
            };

        std::stable_sort(
            result.begin(),
            result.end(),
            [&sorting, &compare](
                const PortalProject& left,
                const PortalProject& right
            ) {
                for (const auto& sort : sorting) {
                    if (sort.field == "title" &&
                        left.title != right.title) {
                        return compare(
                            left.title,
                            right.title,
                            sort.direction
                        );
                    }

                    if (sort.field == "status" &&
                        left.status != right.status) {
                        return compare(
                            left.status,
                            right.status,
                            sort.direction
                        );
                    }

                    if (sort.field == "id" &&
                        left.id != right.id) {
                        return compare(
                            left.id,
                            right.id,
                            sort.direction
                        );
                    }
                }

                return left.id < right.id;
            }
        );

        const auto totalItems =
            static_cast<int>(result.size());

        const auto totalPages =
            std::max(
                1,
                (totalItems + pageSize - 1) / pageSize
            );

        const auto beginIndex =
            static_cast<std::size_t>(page - 1) *
            static_cast<std::size_t>(pageSize);

        const auto endIndex =
            std::min(
                result.size(),
                beginIndex +
                    static_cast<std::size_t>(pageSize)
            );

        Json::Value items(Json::arrayValue);

        if (beginIndex < result.size()) {
            for (auto index = beginIndex;
                 index < endIndex;
                 ++index) {
                items.append(
                    projectJson(result[index])
                );
            }
        }

        Json::Value projectPage(Json::objectValue);
        projectPage["items"] = std::move(items);
        projectPage["page"] = page;
        projectPage["pageSize"] = pageSize;
        projectPage["totalItems"] = totalItems;
        projectPage["totalPages"] = totalPages;

        Json::Value data(Json::objectValue);
        data["projectPage"] = std::move(projectPage);

        return PortalGraphQLOperationSupport::response(data);
    }

    std::shared_ptr<PortalDataset> dataset_;
};