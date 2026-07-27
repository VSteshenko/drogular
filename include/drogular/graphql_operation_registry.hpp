#pragma once

#include <drogular/graphql_execution_context.hpp>
#include <drogular/graphql_response.hpp>
#include <drogular/graphql_variables.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

namespace drogular {

class GraphQLOperationRegistry {
public:
    using Handler = std::function<GraphQLResponse(
        const GraphQLVariables&,
        const GraphQLExecutionContext&)>;

    using LegacyHandler = std::function<GraphQLResponse(
        const GraphQLVariables&)>;

    void registerQuery(std::string name, Handler handler) {
        queryHandlers_.insert_or_assign(
            std::move(name),
            std::move(handler)
        );
    }

    void registerQuery(std::string name, LegacyHandler handler) {
        registerQuery(
            std::move(name),
            [handler = std::move(handler)](
                const GraphQLVariables& variables,
                const GraphQLExecutionContext&
            ) {
                return handler(variables);
            }
        );
    }

    void registerMutation(std::string name, Handler handler) {
        mutationHandlers_.insert_or_assign(
            std::move(name),
            std::move(handler)
        );
    }

    void registerMutation(std::string name, LegacyHandler handler) {
        registerMutation(
            std::move(name),
            [handler = std::move(handler)](
                const GraphQLVariables& variables,
                const GraphQLExecutionContext&
            ) {
                return handler(variables);
            }
        );
    }

    GraphQLResponse executeQuery(
        const std::string& name,
        const GraphQLVariables& variables = {},
        const GraphQLExecutionContext& context = {}
    ) const {
        return execute(queryHandlers_, name, variables, context);
    }

    GraphQLResponse executeMutation(
        const std::string& name,
        const GraphQLVariables& variables = {},
        const GraphQLExecutionContext& context = {}
    ) const {
        return execute(mutationHandlers_, name, variables, context);
    }

private:
    static GraphQLResponse emptyResponse() {
        Json::Value root(Json::objectValue);
        root["data"] = Json::Value(Json::objectValue);
        return GraphQLResponse(root);
    }

    static GraphQLResponse execute(
        const std::unordered_map<std::string, Handler>& handlers,
        const std::string& name,
        const GraphQLVariables& variables,
        const GraphQLExecutionContext& context
    ) {
        const auto iterator = handlers.find(name);
        if (iterator == handlers.end()) {
            return emptyResponse();
        }
        return iterator->second(variables, context);
    }

    std::unordered_map<std::string, Handler> queryHandlers_;
    std::unordered_map<std::string, Handler> mutationHandlers_;
};

} // namespace drogular