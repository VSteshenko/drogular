#include <drogular/graphql_request.hpp>

#include <utility>

namespace drogular {

GraphQLRequest::GraphQLRequest(std::string query)
    : query_(std::move(query)) {
}

void GraphQLRequest::setQuery(std::string query) {
    query_ = std::move(query);
}

const std::string& GraphQLRequest::query() const {
    return query_;
}

void GraphQLRequest::setVariable(
    std::string name,
    Json::Value value
) {
    variables_[std::move(name)] = std::move(value);
}

const Json::Value& GraphQLRequest::variables() const {
    return variables_;
}

GraphQLRequest& GraphQLRequest::query(std::string query) {
    setQuery(std::move(query));
    return *this;
}

GraphQLRequest& GraphQLRequest::variable(
    std::string name,
    Json::Value value
) {
    setVariable(
        std::move(name),
        std::move(value)
    );

    return *this;
}

Json::Value GraphQLRequest::toJson() const {
    Json::Value body(Json::objectValue);

    body["query"] = query_;
    body["variables"] = variables_;

    return body;
}

} // namespace drogular