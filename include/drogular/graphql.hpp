#pragma once

#include <string>
#include <vector>

namespace drogular::gql {

/**
 * Represents a GraphQL literal value.
 */
class Value {
public:
    explicit Value(std::string text);

    /**
     * Converts the value to GraphQL text.
     */
    const std::string& toString() const;

private:
    std::string text_;
};

/**
 * Creates a GraphQL string value.
 */
Value string(std::string value);

/**
 * Creates a GraphQL integer value.
 */
Value intValue(int value);

/**
 * Creates a GraphQL boolean value.
 */
Value boolValue(bool value);

/**
 * Creates a raw GraphQL value.
 *
 * Use this for variables, enums, null, or advanced values.
 */
Value raw(std::string value);

/**
 * Represents a GraphQL argument.
 */
struct Argument {
    std::string name;
    Value value;
};

/**
 * Represents a GraphQL field selection.
 *
 * A field can be scalar, nested, and can contain arguments.
 */
class Selection {
public:
    explicit Selection(std::string name);

    Selection(std::string name, std::vector<Selection> children);

    /**
     * Adds an argument to the field.
     */
    Selection& arg(std::string name, Value value);

    /**
     * Adds nested child fields.
     */
    Selection& children(std::vector<Selection> children);

    /**
     * Converts the selection to GraphQL text.
     */
    std::string toString(unsigned int indent = 2) const;

private:
    std::string name_;
    std::vector<Argument> arguments_;
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
     * Adds a selection to the query.
     */
    Query& select(Selection selection);

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
