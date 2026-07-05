#pragma once

#include "../project_type_provider.hpp"

#include "documents/project_type_queries.hpp"
#include "mappers/project_type_mapper.hpp"

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

private:
    std::shared_ptr<drogular::GraphQLClient> client_;
};