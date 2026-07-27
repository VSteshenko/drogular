#pragma once

#include <drogular/graphql_execution_context.hpp>
#include <drogular/graphql_operation_registry.hpp>
#include <drogular/graphql_response.hpp>
#include <drogular/graphql_variables.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace drogular {

class GraphQLServer {
public:
    GraphQLServer() = default;

    template <typename TOperations, typename... TArguments>
    GraphQLServer& add(TArguments&&... arguments) {
        auto operations = std::make_shared<TOperations>(
            std::forward<TArguments>(arguments)...
        );

        operations->registerWith(registry_);
        operations_.push_back(std::move(operations));
        return *this;
    }

    GraphQLResponse executeQuery(
        const std::string& name,
        const GraphQLVariables& variables = {},
        const GraphQLExecutionContext& context = {}
    ) const {
        return registry_.executeQuery(name, variables, context);
    }

    GraphQLResponse executeMutation(
        const std::string& name,
        const GraphQLVariables& variables = {},
        const GraphQLExecutionContext& context = {}
    ) const {
        return registry_.executeMutation(name, variables, context);
    }

private:
    GraphQLOperationRegistry registry_;
    std::vector<std::shared_ptr<void>> operations_;
};

} // namespace drogular