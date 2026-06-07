#pragma once

#include <string>
#include <vector>

namespace drogular::gql {

/**
 * Represents a GraphQL field selection.
 */
class Selection {
public:
    Selection(std::string name, std::vector<std::string> fields);

    /**
     * Converts the selection to GraphQL text.
     */
    std::string toString(unsigned int indent = 2) const;

private:
    std::string name_;
    std::vector<std::string> fields_;
};

/**
 * Builds a GraphQL query string.
 */
class Query {
public:
    explicit Query(std::string name);

    /**
     * Adds a field selection to the query.
     */
    Query& select(std::string name, std::vector<std::string> fields);

    /**
     * Converts the query to GraphQL text.
     */
    std::string toString() const;

private:
    std::string name_;
    std::vector<Selection> selections_;
};

/**
 * Creates a named GraphQL query.
 */
Query query(std::string name);

} // namespace drogular::gql
