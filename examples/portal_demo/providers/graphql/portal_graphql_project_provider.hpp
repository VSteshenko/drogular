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

    PortalProject create(
        PortalProject project
    ) override {
        const auto response =
            client_->execute(
                ProjectMutations::create(project),
                ProjectMapper::toVariables(project)
            );

        return ProjectMapper::fromValue(
            *response.field("createProject")
        );
    }

    bool update(
        PortalProject project
    ) override {
        const auto response =
            client_->execute(
                ProjectMutations::update(project),
                ProjectMapper::toVariables(project)
            );

        return response.field("updateProject").has_value();
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