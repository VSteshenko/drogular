#pragma once

#include "portal_project.hpp"
#include "portal_user.hpp"

#include <drogular/graphql_client.hpp>

#include <json/value.h>
#include <memory>
#include <vector>

class PortalFixture {
public:
    PortalFixture& addUser(
        PortalUser user
    ) {
        users_.push_back(std::move(user));
        return *this;
    }

    PortalFixture& addProject(
        PortalProject project
    ) {
        projects_.push_back(std::move(project));
        return *this;
    }

    const std::vector<PortalUser>& users() const {
        return users_;
    }

    const std::vector<PortalProject>& projects() const {
        return projects_;
    }

    Json::Value projectGraphQLData() const {
        Json::Value projects(Json::arrayValue);

        for (const auto& project : projects_) {
            projects.append(projectJson(project));
        }

        Json::Value data(Json::objectValue);
        data["projects"] = projects;

        if (!projects_.empty()) {
            data["project"] =
                projectJson(projects_.front());
        } else {
            data["project"] = Json::Value();
        }

        return data;
    }

    std::shared_ptr<drogular::StaticGraphQLClient> graphQLClient() const {
        return std::make_shared<drogular::StaticGraphQLClient>(
            projectGraphQLData()
        );
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

    std::vector<PortalUser> users_;
    std::vector<PortalProject> projects_;
};