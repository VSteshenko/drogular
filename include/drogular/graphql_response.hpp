#pragma once

#include <json/json.h>

namespace drogular {

/**
 * Represents a GraphQL response.
 */
class GraphQLResponse {
public:
    GraphQLResponse() = default;

    explicit GraphQLResponse(
        Json::Value response
    );

    /**
     * Returns GraphQL data object.
     */
    const Json::Value& data() const;

    /**
     * Returns GraphQL errors array.
     */
    const Json::Value& errors() const;

    /**
     * Returns the original response JSON.
     */
    const Json::Value& rawJson() const;

    /**
     * Returns true when GraphQL errors exist.
     */
    bool hasErrors() const;

private:
    Json::Value response_;
};

} // namespace drogular