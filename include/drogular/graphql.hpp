#pragma once

#include <string>
#include <vector>
#include <optional>

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
 * Creates a GraphQL variable reference.
 */
Value variable(std::string name);

/**
 * Represents a GraphQL argument.
 */
struct Argument {
    std::string name;
    Value value;
};

/**
 * Represents the kind of GraphQL selection.
 */
enum class SelectionKind {
    Field,
    FragmentSpread
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
     * Sets an alias for the field.
     */
    Selection& alias(std::string alias);

    /**
     * Adds an argument to the field.
     */
    Selection& arg(std::string name, Value value);

    /**
     * Creates a fragment spread selection.
     */
    static Selection fragmentSpread(std::string name);

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
    std::optional<std::string> alias_;
    std::vector<Argument> arguments_;
    SelectionKind kind_ = SelectionKind::Field;
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
 * Creates a GraphQL fragment spread.
 */
Selection spread(std::string name);

/**
 * Represents a GraphQL fragment definition.
 */
class Fragment {
public:
    Fragment(std::string name, std::string typeName, std::vector<Selection> selections);

    /**
     * Converts the fragment to GraphQL text.
     */
    std::string toString() const;

private:
    std::string name_;
    std::string typeName_;
    std::vector<Selection> selections_;
};

/**
 * Creates a GraphQL fragment definition.
 */
Fragment fragment(std::string name, std::string typeName, std::vector<Selection> selections);

/**
 * Represents a GraphQL variable declaration.
 */
struct Variable {
    std::string name;
    std::string type;
};

/**
 * Builds a GraphQL query string.
 */
class Query {
public:
    explicit Query(std::string name);

    /**
     * Adds a variable declaration to the query.
     */
    Query& variable(std::string name, std::string type);

    /**
     * Adds a selection to the query.
     */
    Query& select(Selection selection);

    /**
     * Adds a field selection to the query.
     */
    Query& select(std::string name, std::vector<Selection> children);

    /**
     * Adds a fragment definition to the query document.
     */
    Query& fragment(Fragment fragment);

    /**
     * Converts the query to GraphQL text.
     */
    std::string toString() const;

private:
    std::string name_;
    std::vector<Variable> variables_;
    std::vector<Selection> selections_;
    std::vector<Fragment> fragments_;
};

/**
 * Creates a named GraphQL query.
 */
Query query(std::string name);

} // namespace drogular::gql
