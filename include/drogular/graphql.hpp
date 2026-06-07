#pragma once

#include <string>

namespace drogular::gql {

/**
 * Placeholder for the future GraphQL query builder.
 */
class Query {
public:
    explicit Query(std::string name);

    /**
     * Converts the query to GraphQL text.
     */
    std::string toString() const;

private:
    std::string name_;
};

} // namespace drogular::gql
