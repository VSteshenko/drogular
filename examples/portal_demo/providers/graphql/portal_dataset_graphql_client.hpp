#pragma once

#include "data/portal_dataset.hpp"
#include "data/portal_schema.hpp"
#include "data/portal_schema_mapper.hpp"
#include "features/projects/data/portal_project_sort.hpp"

#include <drogular/static_graphql_client.hpp>
#include <drogular/graphql_response.hpp>

#include <algorithm>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

static std::string lower(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char character) {
            return static_cast<char>(
                std::tolower(character)
            );
        }
    );

    return value;
}

class PortalDatasetGraphQLClient final
    : public drogular::GraphQLClient
{
    using Handler =
        std::function<drogular::GraphQLResponse(
            const drogular::GraphQLVariables&
        )>;

public:
    explicit PortalDatasetGraphQLClient(
        std::shared_ptr<PortalDataset> dataset
    )
        : dataset_(std::move(dataset))
    {
        registerHandlers();
    }

    drogular::GraphQLResponse execute(
        const drogular::gql::Query& query,
        const drogular::GraphQLVariables& variables = {}
    ) override
    {
        return executeOperation(
            queryHandlers_,
            operationName(
                query.toString(),
                "query"
            ),
            variables
        );
    }

    drogular::GraphQLResponse execute(
        const drogular::gql::Mutation& mutation,
        const drogular::GraphQLVariables& variables = {}
    ) override
    {
        return executeOperation(
            mutationHandlers_,
            operationName(
                mutation.toString(),
                "mutation"
            ),
            variables
        );
    }

    static std::string operationName(
        const std::string& text,
        const std::string& type
    ) {
        const auto prefix =
            type + " ";

        if (!text.starts_with(prefix)) {
            return "";
        }

        const auto nameStart =
            prefix.size();

        const auto nameEnd =
            text.find_first_of(
                " (",
                nameStart
            );

        if (nameEnd == std::string::npos) {
            return text.substr(nameStart);
        }

        return text.substr(
            nameStart,
            nameEnd - nameStart
        );
    }

    void registerHandlers() {
        queryHandlers_["PortalProjects"] =
            [this](const auto&) {
                return projectsResponse();
            };

        queryHandlers_["PortalProjectById"] =
            [this](const auto& variables) {
                return projectResponse(
                    variables.json()["id"].asInt()
                );
            };

        queryHandlers_["PortalUsers"] =
            [this](const auto&) {
                return usersResponse();
            };

        queryHandlers_["PortalUserByCredentials"] =
            [this](const auto& variables) {
                return userByCredentialsResponse(
                    variables.json()["username"].asString(),
                    variables.json()["password"].asString()
                );
            };

        queryHandlers_["PortalRoles"] =
            [this](const auto&) {
                return rolesResponse();
            };

        queryHandlers_["PortalRoleByCode"] =
            [this](const auto& variables) {
                return roleByCodeResponse(
                    variables.json()["code"].asString()
                );
            };

        queryHandlers_["PortalRoleById"] =
            [this](const auto& variables) {
                return roleByIdResponse(
                    variables.json()["id"].asInt()
                );
        };

        queryHandlers_["PortalProjectTypes"] =
            [this](const auto&) {
                return projectTypesResponse();
            };

        queryHandlers_["PortalProjectTypeById"] =
            [this](const auto& variables) {
                return projectTypeResponse(
                    variables.json()["id"].asInt()
                );
            };

        mutationHandlers_["CreatePortalProject"] =
            [this](const auto& variables) {
                return createProjectResponse(
                    variables.json()["project"]
                );
            };

        mutationHandlers_["UpdatePortalProject"] =
            [this](const auto& variables) {
                return updateProjectResponse(
                    variables.json()["project"]
                );
            };

        mutationHandlers_["RemovePortalProject"] =
            [this](const auto& variables) {
                return removeProjectResponse(
                    variables.json()["id"].asInt()
                );
            };

        mutationHandlers_["CreatePortalUser"] =
            [this](const auto& variables) {
                return createUserResponse(
                    variables.json()["user"]
                );
            };

        mutationHandlers_["UpdatePortalUser"] =
            [this](const auto& variables) {
                return updateUserResponse(
                    variables.json()["user"]
                );
            };

        mutationHandlers_["CreatePortalProjectType"] =
            [this](const auto& variables) {
                return createProjectTypeResponse(
                    variables.json()["projectType"]
                );
        };

        mutationHandlers_["UpdatePortalProjectType"] =
            [this](const auto& variables) {
                return updateProjectTypeResponse(
                    variables.json()["projectType"]
                );
        };

        mutationHandlers_["RemovePortalProjectType"] =
            [this](const auto& variables) {
                return removeProjectTypeResponse(
                    variables.json()["id"].asInt()
                );
        };

        mutationHandlers_["CreatePortalRole"] =
            [this](const auto& variables) {
                return createRoleResponse(
                    variables.json()["role"]
                );
        };

        mutationHandlers_["UpdatePortalRole"] =
            [this](const auto& variables) {
                return updateRoleResponse(
                    variables.json()["role"]
                );
        };

        mutationHandlers_["RemovePortalRole"] =
            [this](const auto& variables) {
                return removeRoleResponse(
                    variables.json()["id"].asInt()
                );
        };

        queryHandlers_["SearchPortalProjects"] =
            [this](const auto& variables) {
                const auto& json =
                    variables.json();

                const auto search =
                    json.isMember("search")
                        ? json["search"].asString()
                        : std::string("");

                const auto status =
                    json.isMember("status")
                        ? json["status"].asString()
                        : std::string("");

                const auto projectTypeId =
                    json.isMember("projectTypeId")
                        ? std::optional<int>(
                            json["projectTypeId"].asInt()
                        )
                        : std::nullopt;

                const auto ownerId =
                    json.isMember("ownerId")
                        ? std::optional<int>(
                            json["ownerId"].asInt()
                        )
                        : std::nullopt;

                std::vector<PortalProjectSort> sorting;

                if (json.isMember("sorting") &&
                    json["sorting"].isArray()) {
                    for (const auto& item :
                        json["sorting"]) {
                        const auto field =
                            item["field"].asString();

                        if (field != "id" &&
                            field != "title" &&
                            field != "status") {
                            continue;
                            }

                        const auto direction =
                            item["direction"].asString() ==
                                    "desc"
                                ? PortalProjectSortDirection::
                                      Descending
                                : PortalProjectSortDirection::
                                      Ascending;

                        sorting.push_back({
                            .field = field,
                            .direction = direction
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
        };
    }

    drogular::GraphQLResponse executeRequest(
        const drogular::GraphQLRequest&
    ) override {
        return emptyResponse();
    }

private:
    drogular::GraphQLResponse executeOperation(
        const std::unordered_map<std::string, Handler>& handlers,
        const std::string& name,
        const drogular::GraphQLVariables& variables
    ) const
    {
        const auto handler =
            handlers.find(name);

        if (handler == handlers.end()) {
            return emptyResponse();
        }

        return handler->second(
            variables
        );
    }

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

        return response(data);
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

        return response(data);
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

        return response(data);
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

        return response(data);
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

        return response(data);
    }

    int nextProjectId() const {
        int nextId = 1;

        for (const auto& project : dataset_->projects()) {
            nextId =
                std::max(nextId, project.id + 1);
        }

        return nextId;
    }

    static Json::Value userJson(
        const PortalUser& user
    ) {
        return PortalSchemaMapper::toJson(
            PortalSchema::users(),
            user
        );
    }

    drogular::GraphQLResponse rolesResponse() const {
        Json::Value roles(Json::arrayValue);

        for (const auto& role : dataset_->roles()) {
            roles.append(roleJson(role));
        }

        Json::Value data(Json::objectValue);
        data["roles"] = roles;

        return response(data);
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

        return response(data);
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

        return response(data);
    }

    drogular::GraphQLResponse usersResponse() const {
        Json::Value users(Json::arrayValue);

        for (const auto& user : dataset_->users()) {
            users.append(userJson(user));
        }

        Json::Value data(Json::objectValue);
        data["users"] = users;

        return response(data);
    }

    drogular::GraphQLResponse userByCredentialsResponse(
        const std::string& username,
        const std::string& password
    ) const {
        Json::Value data(Json::objectValue);
        data["userByCredentials"] = Json::Value();

        for (const auto& user : dataset_->users()) {
            if (user.username == username &&
                user.password == password) {
                data["userByCredentials"] =
                    userJson(user);
                break;
                }
        }

        return response(data);
    }

    int nextUserId() const {
        int nextId = 1;

        for (const auto& user : dataset_->users()) {
            nextId = std::max(nextId, user.id + 1);
        }

        return nextId;
    }

    drogular::GraphQLResponse createUserResponse(
        const Json::Value& value
    ) {
        auto user =
            PortalSchemaMapper::fromJson(
                PortalSchema::users(),
                value
            );

        user.id =
            nextUserId();

        dataset_->users().push_back(user);

        Json::Value data(Json::objectValue);
        data["createUser"] = userJson(user);

        return response(data);
    }

    drogular::GraphQLResponse updateUserResponse(
        const Json::Value& value
    ) {
        Json::Value data(Json::objectValue);
        data["updateUser"] = Json::Value();

        const auto id =
            value["id"].asInt();

        for (auto& user : dataset_->users()) {
            if (user.id != id) {
                continue;
            }

            if (value.isMember("username")) {
                user.username =
                    value["username"].asString();
            }

            if (value.isMember("role")) {
                user.role =
                    value["role"].asString();
            }

            data["updateUser"] =
                userJson(user);

            break;
        }

        return response(data);
    }

    static drogular::GraphQLResponse response(
        Json::Value data
    ) {
        Json::Value root(Json::objectValue);
        root["data"] = std::move(data);

        return drogular::GraphQLResponse(root);
    }

    static drogular::GraphQLResponse emptyResponse() {
        return response(Json::Value(Json::objectValue));
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

        return response(data);
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

        return response(data);
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

        return response(data);
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

        return response(data);
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
            return response(data);
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

        return response(data);
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

        return response(data);
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

        return response(data);
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
            return response(data);
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
            return response(data);
        }

        dataset_->roles().erase(role);

        data["removeRole"] = true;

        return response(data);
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
            lower(search);

        std::vector<PortalProject> result;

        for (const auto& project :
            dataset_->projects()) {
            if (!needle.empty() &&
                lower(project.title).find(needle) ==
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
                .direction =
                    PortalProjectSortDirection::
                        Ascending
            });
        }

        const auto compare =
            []<typename T>(
                const T& left,
                const T& right,
                PortalProjectSortDirection direction
            ) {
                return direction ==
                       PortalProjectSortDirection::
                           Ascending
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

        return response(data);
    }

    std::shared_ptr<PortalDataset> dataset_;
    std::unordered_map<std::string, Handler> queryHandlers_;
    std::unordered_map<std::string, Handler> mutationHandlers_;
};