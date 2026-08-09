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
    : DrogularError(message) {
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
}

} // namespace drogular