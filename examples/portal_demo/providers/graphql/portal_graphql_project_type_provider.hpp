#pragma once

#include "providers/project_type_provider.hpp"
#include "providers/graphql/documents/project_type_queries.hpp"
#include "providers/graphql/documents/project_type_mutations.hpp"
#include "providers/graphql/mappers/project_type_mapper.hpp"

#include <drogular/graphql_client.hpp>

#include <memory>
#include <optional>
#include <vector>

class PortalGraphQLProjectTypeProvider final
    : public PortalProjectTypeProvider
{
public:
    explicit PortalGraphQLProjectTypeProvider(
        std::shared_ptr<drogular::GraphQLClient> client
    )
        : client_(std::move(client))
    {
    }

    std::vector<PortalProjectType> all() const override {
        const auto response =
            client_->execute(ProjectTypeQueries::all());

        const auto values =
            response.field("projectTypes");

        if (!values.has_value()) {
            return {};
        }

        return ProjectTypeMapper::fromList(*values);
    }

    std::optional<PortalProjectType> findById(
        int id
    ) const override {
        const auto response =
            client_->execute(
                ProjectTypeQueries::findById(),
                ProjectTypeMapper::idVariables(id)
            );

        return ProjectTypeMapper::optionalType(
            response.field("projectType")
        );
    }

    PortalProjectType create(
        const PortalProjectTypeCreate& input
    ) override {
        const auto response =
            client_->execute(
                ProjectTypeMutations::create(input),
                ProjectTypeMapper::toVariables(input)
            );

        const auto value =
            response.field("createProjectType");

        if (!value.has_value()) {
            return {};
        }

        return ProjectTypeMapper::fromValue(*value);
    }

    PortalProjectType update(
        const PortalProjectTypeUpdate& input
    ) override {
        const auto response =
            client_->execute(
                ProjectTypeMutations::update(input),
                ProjectTypeMapper::toVariables(input)
            );

        const auto value =
            response.field("updateProjectType");

        if (!value.has_value()) {
            return {};
        }

        return ProjectTypeMapper::fromValue(*value);
    }

    bool remove(
        int id
    ) override {
        const auto response =
            client_->execute(
                ProjectTypeMutations::remove(),
                ProjectTypeMapper::idVariables(id)
            );

        const auto value =
            response.field("removeProjectType");

        return value.has_value() &&
               value->asBool();
    }

private:
    std::shared_ptr<drogular::GraphQLClient> client_;
};