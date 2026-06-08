#include <drogular/graphql_client.hpp>

#include <drogon/drogon.h>

#include <future>
#include <stdexcept>
#include <utility>

namespace drogular {

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

GraphQLResult HttpGraphQLClient::execute(const gql::Query& query) {
    auto client = drogon::HttpClient::newHttpClient(
        "http://" + host_ + ":" + std::to_string(port_)
    );

    Json::Value body;
    body["query"] = query.toString();

    auto request = drogon::HttpRequest::newHttpJsonRequest(body);
    request->setMethod(drogon::Post);
    request->setPath(path_);

    std::promise<drogon::HttpResponsePtr> promise;
    auto future = promise.get_future();

    client->sendRequest(
        request,
        [&promise](drogon::ReqResult result, const drogon::HttpResponsePtr& response) {
            if (result != drogon::ReqResult::Ok || response == nullptr) {
                promise.set_exception(
                    std::make_exception_ptr(
                        std::runtime_error("GraphQL HTTP request failed")
                    )
                );
                return;
            }

            promise.set_value(response);
        }
    );

    const auto response = future.get();

    if (response->statusCode() < 200 || response->statusCode() >= 300) {
        throw std::runtime_error("GraphQL HTTP response returned non-success status");
    }

    GraphQLResult result;
    result.set("__json", response->body());

    return result;
}

} // namespace drogular