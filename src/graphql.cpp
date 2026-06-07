#include <drogular/graphql.hpp>

#include <sstream>
#include <utility>

namespace drogular::gql {

namespace {

/**
 * Creates a string with the requested number of spaces.
 */
std::string spaces(unsigned int count) {
    return std::string(count, ' ');
}

/**
 * Converts GraphQL variable declarations to text.
 */
std::string variablesToString(const std::vector<Variable>& variables) {
    if (variables.empty()) {
        return "";
    }

    std::ostringstream output;

    output << "(";

    for (size_t i = 0; i < variables.size(); ++i) {
        output << "$" << variables[i].name << ": " << variables[i].type;

        if (i + 1 < variables.size()) {
            output << ", ";
        }
    }

    output << ")";

    return output.str();
}

/**
 * Converts GraphQL arguments to text.
 */
std::string argumentsToString(const std::vector<Argument>& arguments) {
    if (arguments.empty()) {
        return "";
    }

    std::ostringstream output;

    output << "(";

    for (size_t i = 0; i < arguments.size(); ++i) {
        output << arguments[i].name << ": " << arguments[i].value.toString();

        if (i + 1 < arguments.size()) {
            output << ", ";
        }
    }

    output << ")";

    return output.str();
}

} // namespace

Value::Value(std::string text)
    : text_(std::move(text)) {
}

const std::string& Value::toString() const {
    return text_;
}

Value string(std::string value) {
    std::string escaped;
    escaped.reserve(value.size() + 2);

    escaped += '"';

    for (const auto ch : value) {
        if (ch == '"' || ch == '\\') {
            escaped += '\\';
        }

        escaped += ch;
    }

    escaped += '"';

    return Value(std::move(escaped));
}

Value intValue(int value) {
    return Value(std::to_string(value));
}

Value boolValue(bool value) {
    return Value(value ? "true" : "false");
}

Value raw(std::string value) {
    return Value(std::move(value));
}

Value variable(std::string name) {
    return Value("$" + std::move(name));
}

Selection::Selection(std::string name)
    : name_(std::move(name)) {
}

Selection::Selection(std::string name, std::vector<Selection> children)
    : name_(std::move(name)),
      children_(std::move(children)) {
}

Selection& Selection::arg(std::string name, Value value) {
    arguments_.push_back({
        .name = std::move(name),
        .value = std::move(value)
    });

    return *this;
}

Selection& Selection::children(std::vector<Selection> children) {
    children_ = std::move(children);
    return *this;
}

std::string Selection::toString(unsigned int indent) const {
    std::ostringstream output;

    output << spaces(indent) << name_;
    output << argumentsToString(arguments_);

    if (children_.empty()) {
        return output.str();
    }

    output << " {\n";

    for (const auto& child : children_) {
        output << child.toString(indent + 2) << "\n";
    }

    output << spaces(indent) << "}";

    return output.str();
}

Selection field(std::string name) {
    return Selection(std::move(name));
}

Selection field(std::string name, std::vector<Selection> children) {
    return Selection(std::move(name), std::move(children));
}

Query::Query(std::string name)
    : name_(std::move(name)) {
}

Query& Query::variable(std::string name, std::string type) {
    variables_.push_back({
        .name = std::move(name),
        .type = std::move(type)
    });

    return *this;
}

Query& Query::select(Selection selection) {
    selections_.push_back(std::move(selection));
    return *this;
}

Query& Query::select(std::string name, std::vector<Selection> children) {
    selections_.emplace_back(std::move(name), std::move(children));
    return *this;
}

std::string Query::toString() const {
    std::ostringstream output;

    output << "query " << name_ << variablesToString(variables_) << " {\n";

    for (const auto& selection : selections_) {
        output << selection.toString(2) << "\n";
    }

    output << "}";

    return output.str();
}

Query query(std::string name) {
    return Query(std::move(name));
}

} // namespace drogular::gql
