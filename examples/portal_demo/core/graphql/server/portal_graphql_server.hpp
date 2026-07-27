#pragma once

#include "core/graphql/server/portal_graphql_operation_registry.hpp"

#include <drogular/graphql_response.hpp>
#include <drogular/graphql_variables.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

class PortalGraphQLServer {
public:
    PortalGraphQLServer() = default;

    template <typename TOperations, typename... TArguments>
    PortalGraphQLServer& add(TArguments&&... arguments) {
        auto operations = std::make_shared<TOperations>(
            std::forward<TArguments>(arguments)...
        );

        operations->registerWith(registry_);
        operations_.push_back(std::move(operations));
        return *this;
    }

    drogular::GraphQLResponse executeQuery(
        const std::string& name,
        const drogular::GraphQLVariables& variables = {},
        const PortalGraphQLExecutionContext& context = {}
    ) const {
        return registry_.executeQuery(name, variables, context);
    }

    drogular::GraphQLResponse executeMutation(
        const std::string& name,
        const drogular::GraphQLVariables& variables = {},
        const PortalGraphQLExecutionContext& context = {}
    ) const {
        return registry_.executeMutation(name, variables, context);
    }

private:
    PortalGraphQLOperationRegistry registry_;
    std::vector<std::shared_ptr<void>> operations_;
};