#include <drogular/graphql_response.hpp>

namespace drogular {

GraphQLResponse::GraphQLResponse(
    Json::Value response
)
    : response_(std::move(response)) {
}

const Json::Value&
GraphQLResponse::data() const {
    static const Json::Value empty;

    if (!response_.isMember("data")) {
        return empty;
    }

    return response_["data"];
}

const Json::Value&
GraphQLResponse::errors() const {
    static const Json::Value empty;

    if (!response_.isMember("errors")) {
        return empty;
    }

    return response_["errors"];
}

const Json::Value&
GraphQLResponse::rawJson() const {
    return response_;
}

bool GraphQLResponse::hasErrors() const {
    return response_.isMember("errors") &&
           response_["errors"].isArray() &&
           !response_["errors"].empty();
}

} // namespace drogular