#pragma once

#include "../project_provider.hpp"
#include "../user_provider.hpp"
#include "documents/project_queries.hpp"
#include "documents/project_mutations.hpp"
#include "mappers/project_mapper.hpp"

#include <drogular/graphql.hpp>
#include <drogular/graphql_client.hpp>
#include <drogular/graphql_variables.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

class PortalGraphQLProjectProvider final
    : public PortalProjectProvider
{
public:
    explicit PortalGraphQLProjectProvider(
        std::shared_ptr<drogular::GraphQLClient> client,
        std::shared_ptr<PortalUserProvider> users
    )
        : client_(std::move(client)),
          users_(std::move(users))
    {
    }

    std::vector<PortalProject> all() const override {
        const auto response =
            client_->execute(
                ProjectQueries::all()
            );

        const auto projects =
            response.field("projects");

        if (!projects.has_value()) {
            return {};
        }

        return ProjectMapper::fromList(*projects);
    }

    std::optional<PortalProject> findById(
        int id
    ) const override {
        const auto response =
            client_->execute(
                ProjectQueries::findById(id),
                ProjectMapper::idVariables(id)
            );

        return ProjectMapper::optionalProject(
            response.field("project")
        );
    }

    PortalPage<PortalProject> search(
        const PortalProjectFilter& filter
    ) const override {
        const auto response =
            client_->execute(
                ProjectQueries::search(filter),
                ProjectMapper::toVariables(filter)
            );

        const auto page =
            response.field("projectPage");

        if (!page.has_value()) {
            return {};
        }

        return ProjectMapper::pageFromValue(
            *page
        );
    }

    PortalProject create(
        const PortalProjectCreate& input,
        int ownerId
    ) override {
        PortalProject project;

        project.title = input.title;
        project.status = input.status.value_or("active");
        project.ownerId = ownerId;
        project.projectTypeId = input.projectTypeId;

        const auto variables =
            ProjectMapper::toVariables(project);

        const auto response =
            client_->execute(
                ProjectMutations::create(project),
                variables
            );

        return ProjectMapper::fromValue(
            response.data()["createProject"]
        );
    }

    PortalProject update(
        const PortalProjectUpdate& input
    ) override {
        const auto variables =
            ProjectMapper::toVariables(input);

        const auto response =
            client_->execute(
                ProjectMutations::update(input),
                variables
            );

        return ProjectMapper::fromValue(
            response.data()["updateProject"]
        );
    }

    bool remove(
        int id
    ) override {
        const auto response =
            client_->execute(
                ProjectMutations::remove(id),
                ProjectMapper::idVariables(id)
            );

        const auto removed =
            response.field("removeProject");

        return removed.has_value() &&
               removed->asBool();
    }

    std::optional<PortalUser> owner(
        const PortalProject& project
    ) const override {
        return users_->findById(project.ownerId);
    }

private:
    std::shared_ptr<drogular::GraphQLClient> client_;
    std::shared_ptr<PortalUserProvider> users_;
};