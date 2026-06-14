#pragma once

#include <json/json.h>

#include <string>

namespace drogular {

/**
 * Represents a GraphQL HTTP request body.
 */
class GraphQLRequest {
public:
    explicit GraphQLRequest(std::string query = {});

    /**
     * Sets GraphQL query text.
     */
    void setQuery(std::string query);

    /**
     * Returns GraphQL query text.
     */
    const std::string& query() const;

    /**
     * Sets GraphQL variable value.
     */
    void setVariable(std::string name, Json::Value value);

    /**
     * Returns GraphQL variables object.
     */
    const Json::Value& variables() const;

    /**
     * Converts the request to JSON body.
     */
    Json::Value toJson() const;

private:
    std::string query_;
    Json::Value variables_ = Json::objectValue;
};

} // namespace drogular