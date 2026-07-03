#pragma once

#include "../../data/portal_dataset.hpp"

#include <drogular/graphql_client.hpp>
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

        if (text.find("projects") != std::string::npos) {
            return projectsResponse();
        }

        if (text.find("project") != std::string::npos) {
            return projectResponse(
                variables.json()["id"].asInt()
            );
        }

        if (text.find("userByCredentials") != std::string::npos) {
            return userByCredentialsResponse(
                variables.json()["username"].asString(),
                variables.json()["password"].asString()
            );
        }

        if (text.find("users") != std::string::npos) {
            return usersResponse();
        }

        return emptyResponse();
    }

    drogular::GraphQLResponse execute(
        const drogular::gql::Mutation& mutation,
        const drogular::GraphQLVariables& variables = {}
    ) override {
        const auto text =
            mutation.toString();

        if (text.find("createProject") != std::string::npos) {
            return createProjectResponse(
                variables.json()["project"]
            );
        }

        if (text.find("updateProject") != std::string::npos) {
            return updateProjectResponse(
                variables.json()["project"]
            );
        }

        if (text.find("removeProject") != std::string::npos) {
            return removeProjectResponse(
                variables.json()["id"].asInt()
            );
        }

        if (text.find("createUser") != std::string::npos) {
            return createUserResponse(
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
    static Json::Value projectJson(
        const PortalProject& project
    ) {
        Json::Value value(Json::objectValue);

        value["id"] = project.id;
        value["title"] = project.title;
        value["status"] = project.status;

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

    drogular::GraphQLResponse createUserResponse(
        const Json::Value& value
    ) {
        PortalUser user;

        user.username = value["username"].asString();
        user.password = value["password"].asString();
        user.role = value["role"].asString();

        dataset_->users().push_back(user);

        Json::Value data(Json::objectValue);
        data["createUser"] = userJson(user);

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

    std::shared_ptr<PortalDataset> dataset_;
};