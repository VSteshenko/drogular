#pragma once

#include <drogular/graphql_response.hpp>
#include <drogular/graphql_variables.hpp>

#include <functional>
#include <string>
#include <unordered_map>

class PortalGraphQLOperationRegistry {
public:
    using Handler = std::function<drogular::GraphQLResponse(
        const drogular::GraphQLVariables&)>;

    void registerQuery(std::string name, Handler handler) {
        queryHandlers_.insert_or_assign(
            std::move(name),
            std::move(handler)
        );
    }

    void registerMutation(std::string name, Handler handler) {
        mutationHandlers_.insert_or_assign(
            std::move(name),
            std::move(handler)
        );
    }

    drogular::GraphQLResponse executeQuery(
        const std::string& name,
        const drogular::GraphQLVariables& variables
    ) const {
        return execute(queryHandlers_, name, variables);
    }

    drogular::GraphQLResponse executeMutation(
        const std::string& name,
        const drogular::GraphQLVariables& variables
    ) const {
        return execute(mutationHandlers_, name, variables);
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
        const drogular::GraphQLVariables& variables
    ) {
        const auto iterator = handlers.find(name);
        if (iterator == handlers.end()) {
            return emptyResponse();
        }
        return iterator->second(variables);
    }

    std::unordered_map<std::string, Handler> queryHandlers_;
    std::unordered_map<std::string, Handler> mutationHandlers_;
};