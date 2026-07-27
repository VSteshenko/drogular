#pragma once

#include "core/graphql/server/portal_graphql_execution_context.hpp"

#include <drogular/graphql_response.hpp>
#include <drogular/graphql_variables.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

class PortalGraphQLOperationRegistry {
public:
    using Handler = std::function<drogular::GraphQLResponse(
        const drogular::GraphQLVariables&,
        const PortalGraphQLExecutionContext&)>;

    using LegacyHandler = std::function<drogular::GraphQLResponse(
        const drogular::GraphQLVariables&)>;

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
                const drogular::GraphQLVariables& variables,
                const PortalGraphQLExecutionContext&
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
                const drogular::GraphQLVariables& variables,
                const PortalGraphQLExecutionContext&
            ) {
                return handler(variables);
            }
        );
    }

    drogular::GraphQLResponse executeQuery(
        const std::string& name,
        const drogular::GraphQLVariables& variables,
        const PortalGraphQLExecutionContext& context = {}
    ) const {
        return execute(queryHandlers_, name, variables, context);
    }

    drogular::GraphQLResponse executeMutation(
        const std::string& name,
        const drogular::GraphQLVariables& variables,
        const PortalGraphQLExecutionContext& context = {}
    ) const {
        return execute(mutationHandlers_, name, variables, context);
    }

private:
    static drogular::GraphQLResponse emptyResponse() {
        Json::Value root(Json::objectValue);
        root["data"] = Json::Value(Json::objectValue);
        return drogular::GraphQLResponse(root);
    }

    static drogular::GraphQLResponse execute(
        const std::unordered_map<std::string, Handler>& handlers,
        const std::string& name,
        const drogular::GraphQLVariables& variables,
        const PortalGraphQLExecutionContext& context
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