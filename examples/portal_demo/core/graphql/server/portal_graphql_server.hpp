#pragma once

#include "core/graphql/server/portal_graphql_operation_registry.hpp"
#include "data/portal_dataset.hpp"

#include <drogular/graphql_response.hpp>
#include <drogular/graphql_variables.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

class PortalGraphQLServer {
public:
    explicit PortalGraphQLServer(
        std::shared_ptr<PortalDataset> dataset
    )
        : dataset_(std::move(dataset))
    {
    }

    template <typename TOperations>
    PortalGraphQLServer& add() {
        auto operations =
            std::make_shared<TOperations>(dataset_);

        operations->registerWith(registry_);
        operations_.push_back(std::move(operations));
        return *this;
    }

    drogular::GraphQLResponse executeQuery(
        const std::string& name,
        const drogular::GraphQLVariables& variables = {}
    ) const {
        return registry_.executeQuery(name, variables);
    }

    drogular::GraphQLResponse executeMutation(
        const std::string& name,
        const drogular::GraphQLVariables& variables = {}
    ) const {
        return registry_.executeMutation(name, variables);
    }

private:
    std::shared_ptr<PortalDataset> dataset_;
    PortalGraphQLOperationRegistry registry_;
    std::vector<std::shared_ptr<void>> operations_;
};