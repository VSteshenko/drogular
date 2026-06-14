#include <drogular/graphql_client.hpp>
#include "drogular/graphql.hpp"

#include <drogon/drogon.h>

#include <future>
#include <stdexcept>
#include <utility>

namespace drogular {

GraphQLResponse StaticGraphQLClient::executeRequest(
    const GraphQLRequest&
) {
    Json::Value response(Json::objectValue);

    Json::Value data(Json::objectValue);

    // StaticGraphQLClient keeps GraphQLResult compatibility.
    // It does not provide structured GraphQLResponse data yet.
    response["data"] = data;

    return GraphQLResponse(response);
}

StaticGraphQLClient::StaticGraphQLClient(GraphQLResult result)
    : result_(std::move(result)) {
}

GraphQLResult StaticGraphQLClient::execute(const gql::Query&) {
    return result_;
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

GraphQLResult HttpGraphQLClient::execute(
    const gql::Query& query
) {
    GraphQLRequest request(query.toString());

    const auto response =
        executeRequest(request);

    GraphQLResult result;

    result.set("__json", response.rawJson());

    if (response.hasData()) {
        for (const auto& name : response.data().getMemberNames()) {
            result.set(name, response.data()[name]);
        }
    }

    return result;
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
                        std::runtime_error(
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
        throw std::runtime_error(
            "GraphQL HTTP response returned non-success status"
        );
        }

    const auto json = response->getJsonObject();

    if (json == nullptr) {
        throw std::runtime_error(
            "GraphQL HTTP response is not valid JSON"
        );
    }

    return GraphQLResponse(*json);
}

} // namespace drogular