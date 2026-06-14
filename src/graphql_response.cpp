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

bool GraphQLResponse::hasData() const {
    return response_.isMember("data") &&
           !response_["data"].isNull();
}

std::optional<Json::Value> GraphQLResponse::field(
    const std::string& name
) const {
    if (!hasData()) {
        return std::nullopt;
    }

    if (!response_["data"].isObject() ||
        !response_["data"].isMember(name)) {
        return std::nullopt;
    }

    return response_["data"][name];
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

std::vector<std::string> GraphQLResponse::errorMessages() const {
    std::vector<std::string> messages;

    if (!hasErrors()) {
        return messages;
    }

    for (const auto& error : response_["errors"]) {
        if (error.isString()) {
            messages.push_back(error.asString());
        } else if (error.isObject() &&
                   error.isMember("message") &&
                   error["message"].isString()) {
            messages.push_back(error["message"].asString());
        }
    }

    return messages;
}

GraphQLResult GraphQLResponse::toResult() const {
    GraphQLResult result;

    result.set("__json", rawJson());

    if (hasData() && data().isObject()) {
        for (const auto& name : data().getMemberNames()) {
            result.set(
                name,
                data()[name]
            );
        }
    }

    return result;
}

} // namespace drogular