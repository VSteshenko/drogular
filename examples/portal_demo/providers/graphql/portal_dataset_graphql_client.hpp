#pragma once

#include "../../data/portal_dataset.hpp"

#include <drogular/static_graphql_client.hpp>
#include <drogular/graphql_response.hpp>

#include <algorithm>
#include <memory>
#include <string>

class PortalDatasetGraphQLClient final
    : public drogular::GraphQLClient
{
public:
    explicit PortalDatasetGraphQLClient(
        std::shared_ptr<PortalDataset> dataset
    )
        : dataset_(std::move(dataset))
    {
    }

    drogular::GraphQLResponse execute(
        const drogular::gql::Query& query,
        const drogular::GraphQLVariables& variables = {}
    ) override {
        const auto text =
            query.toString();

        if (isQuery(text, "PortalProjectTypes")) {
            return projectTypesResponse();
        }

        if (isQuery(text, "PortalProjectTypeById")) {
            return projectTypeResponse(
                variables.json()["id"].asInt()
            );
        }

        if (isQuery(text, "PortalProjects")) {
            return projectsResponse();
        }

        if (isQuery(text, "PortalProjectById")) {
            return projectResponse(
                variables.json()["id"].asInt()
            );
        }

        if (isQuery(text, "PortalUsers")) {
            return usersResponse();
        }

        if (isQuery(text, "PortalUserByCredentials")) {
            return userByCredentialsResponse(
                variables.json()["username"].asString(),
                variables.json()["password"].asString()
            );
        }

        return emptyResponse();
    }

    drogular::GraphQLResponse execute(
        const drogular::gql::Mutation& mutation,
        const drogular::GraphQLVariables& variables = {}
    ) override {
        const auto text =
            mutation.toString();

        if (isMutation(text, "CreatePortalProject")) {
            return createProjectResponse(
                variables.json()["project"]
            );
        }

        if (isMutation(text, "UpdatePortalProject")) {
            return updateProjectResponse(
                variables.json()["project"]
            );
        }

        if (isMutation(text, "RemovePortalProject")) {
            return removeProjectResponse(
                variables.json()["id"].asInt()
            );
        }

        if (isMutation(text, "CreatePortalUser")) {
            return createUserResponse(
                variables.json()["user"]
            );
        }

        if (isMutation(text, "UpdatePortalUser")) {
            return updateUserResponse(
                variables.json()["user"]
            );
        }

        return emptyResponse();
    }

    drogular::GraphQLResponse executeRequest(
        const drogular::GraphQLRequest&
    ) override {
        return emptyResponse();
    }

private:
    static bool hasOperationName(
        const std::string& text,
        const std::string& type,
        const std::string& name
    ) {
        return text.starts_with(
            type + " " + name
        );
    }

    static bool isQuery(
        const std::string& text,
        const std::string& name
    ) {
        return hasOperationName(
            text,
            "query",
            name
        );
    }

    static bool isMutation(
        const std::string& text,
        const std::string& name
    ) {
        return hasOperationName(
            text,
            "mutation",
            name
        );
    }

    static Json::Value projectJson(
        const PortalProject& project
    ) {
        Json::Value value(Json::objectValue);

        value["id"] = project.id;
        value["title"] = project.title;
        value["status"] = project.status;
        value["ownerId"] = project.ownerId;
        value["projectTypeId"] = project.projectTypeId;

        return value;
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
        PortalProject project;

        project.id =
            nextProjectId();
        project.title =
            value["title"].asString();
        project.status =
            value["status"].asString();
        project.ownerId =
            value["ownerId"].asInt();
        project.projectTypeId =
            value["projectTypeId"].asInt();

        dataset_->projects().push_back(project);

        Json::Value data(Json::objectValue);
        data["createProject"] = projectJson(project);

        return response(data);
    }

    drogular::GraphQLResponse updateProjectResponse(
        const Json::Value& value
    ) {
        const auto id =
            value["id"].asInt();

        Json::Value data(Json::objectValue);
        data["updateProject"] = Json::Value();

        for (auto& project : dataset_->projects()) {
            if (project.id == id) {
                project.title =
                    value["title"].asString();
                project.status =
                    value["status"].asString();
                project.ownerId =
                    value["ownerId"].asInt();
                project.projectTypeId =
                    value["projectTypeId"].asInt();

                data["updateProject"] =
                    projectJson(project);

                break;
            }
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
        Json::Value value(Json::objectValue);

        value["id"] = user.id;
        value["username"] = user.username;
        value["password"] = user.password;
        value["role"] = user.role;

        return value;
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
        PortalUser user;

        user.id = nextUserId();
        user.username = value["username"].asString();
        user.password = value["password"].asString();
        user.role = value["role"].asString();

        dataset_->users().push_back(user);

        Json::Value data(Json::objectValue);
        data["createUser"] = userJson(user);

        return response(data);
    }

    drogular::GraphQLResponse updateUserResponse(
        const Json::Value& value
    ) {
        const auto id =
            value["id"].asInt();

        Json::Value data(Json::objectValue);
        data["updateUser"] = Json::Value();

        for (auto& user : dataset_->users()) {
            if (user.id == id) {
                user.username = value["username"].asString();
                user.password = value["password"].asString();
                user.role = value["role"].asString();

                data["updateUser"] =
                    userJson(user);

                break;
            }
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
        Json::Value value(Json::objectValue);

        value["id"] = type.id;
        value["code"] = type.code;
        value["title"] = type.title;

        return value;
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

    std::shared_ptr<PortalDataset> dataset_;
};