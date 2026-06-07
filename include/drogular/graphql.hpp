#pragma once

#include <string>
#include <vector>

namespace drogular::gql {

/**
 * Represents a GraphQL field selection.
 *
 * A field can be either scalar or nested.
 */
class Selection {
public:
    explicit Selection(std::string name);

    Selection(std::string name, std::vector<Selection> children);

    /**
     * Converts the selection to GraphQL text.
     */
    std::string toString(unsigned int indent = 2) const;

private:
    std::string name_;
    std::vector<Selection> children_;
};

/**
 * Creates a scalar GraphQL field.
 */
Selection field(std::string name);

/**
 * Creates a nested GraphQL field.
 */
Selection field(std::string name, std::vector<Selection> children);

/**
 * Builds a GraphQL query string.
 */
class Query {
public:
    explicit Query(std::string name);

    /**
     * Adds a field selection to the query.
     */
    Query& select(std::string name, std::vector<Selection> children);

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
