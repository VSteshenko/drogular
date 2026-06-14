#pragma once

#include <drogular/component.hpp>

#include <optional>
#include <string>
#include <vector>
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
     * Returns true when response contains a data object.
     */
    bool hasData() const;

    /**
     * Returns data field by name.
     */
    std::optional<Json::Value> field(const std::string& name) const;

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

    /**
     * Returns GraphQL error messages.
     */
    std::vector<std::string> errorMessages() const;

    /**
     * Converts GraphQL response data to GraphQLResult.
     */
    GraphQLResult toResult() const;

private:
    Json::Value response_;
};

} // namespace drogular