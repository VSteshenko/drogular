#include <drogular/graphql_client.hpp>
#include <drogular/graphql.hpp>
#include <drogular/graphql_response.hpp>

#include <drogon/drogon.h>

#include <future>
#include <stdexcept>
#include <utility>

namespace drogular {

GraphQLClientError::GraphQLClientError(
    const std::string& message
)
    : std::runtime_error(message) {
}

GraphQLResponse GraphQLClient::execute(
    const gql::Query& query,
    const GraphQLVariables& variables
) {
    GraphQLRequest request(
        query.toString()
    );

    request.variables(variables);

    return executeRequest(request);
}

GraphQLResponse GraphQLClient::execute(
    const gql::Mutation& mutation,
    const GraphQLVariables& variables
) {
    GraphQLRequest request(
        mutation.toString()
    );

    request.variables(variables);

    return executeRequest(request);
}

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

HttpGraphQLClient::HttpGraphQLClient(
    std::string host,
    std::uint16_t port,
    std::string path
)
    : host_(std::move(host)),
      port_(port),
      path_(std::move(path)) {
}

GraphQLResponse HttpGraphQLClient::executeRequest(
    const GraphQLRequest& graphQLRequest
) {
    auto client = drogon::HttpClient::newHttpClient(
        "http://" + host_ + ":" + std::to_string(port_)
    );

    auto request =
        drogon::HttpRequest::newHttpJsonRequest(
            graphQLRequest.toJson()
        );

    request->setMethod(drogon::Post);
    request->setPath(path_);

    std::promise<drogon::HttpResponsePtr> promise;
    auto future = promise.get_future();

    client->sendRequest(
        request,
        [&promise](
            drogon::ReqResult result,
            const drogon::HttpResponsePtr& response
        ) {
            if (result != drogon::ReqResult::Ok ||
                response == nullptr) {
                promise.set_exception(
                    std::make_exception_ptr(
                        GraphQLClientError(
                            "GraphQL HTTP request failed"
                        )
                    )
                );
                return;
            }

            promise.set_value(response);
        }
    );

    const auto response = future.get();

    if (response->statusCode() < 200 ||
        response->statusCode() >= 300) {
        throw GraphQLClientError(
            "GraphQL HTTP response returned non-success status"
        );
    }

    const auto json = response->getJsonObject();

    if (json == nullptr) {
        throw GraphQLClientError(
            "GraphQL HTTP response is not valid JSON"
        );
    }

    return GraphQLResponse(*json);
/*    GraphQLResponse graphQLResponse(*json);

    if (graphQLResponse.hasErrors()) {
        throw GraphQLClientError(
            "GraphQL response contains errors"
        );
    }

    return graphQLResponse;*/
}

} // namespace drogular