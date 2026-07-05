#include <drogular/static_graphql_client.hpp>

#include <drogon/drogon.h>

#include <utility>

namespace drogular {

StaticGraphQLClient::StaticGraphQLClient(
    Json::Value data
) {
    Json::Value response(Json::objectValue);
    response["data"] = std::move(data);

    response_ = GraphQLResponse(response);
}

GraphQLResponse StaticGraphQLClient::execute(
    const gql::Query& query,
    const GraphQLVariables& variables
) {
    GraphQLRequest request(
        query.toString()
    );

    request.variables(variables);

    return executeRequest(request);
}

GraphQLResponse StaticGraphQLClient::execute(
    const gql::Mutation& mutation,
    const GraphQLVariables& variables
) {
    GraphQLRequest request(
        mutation.toString()
    );

    request.variables(variables);

    return executeRequest(request);
}

GraphQLResponse StaticGraphQLClient::executeRequest(
    const GraphQLRequest& request
) {
    requests_.push_back(request);

    return response_;
}

const std::vector<GraphQLRequest>& StaticGraphQLClient::requests() const {
    return requests_;
}

std::optional<GraphQLRequest> StaticGraphQLClient::lastRequest() const {
    if (requests_.empty()) {
        return std::nullopt;
    }

    return requests_.back();
}

void StaticGraphQLClient::clearRequests() {
    requests_.clear();
}

std::size_t StaticGraphQLClient::requestCount() const {
    return requests_.size();
}

} // namespace drogular