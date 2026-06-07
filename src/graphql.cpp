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

/**
 * Collects fragment spread names from a selection tree.
 */
void collectFragmentSpreads(
    const Selection& selection,
    std::set<std::string>& spreads
) {
    if (selection.kind() == SelectionKind::FragmentSpread) {
        spreads.insert(selection.name());
        return;
    }

    for (const auto& child : selection.children()) {
        collectFragmentSpreads(child, spreads);
    }
}

/**
 * Validates a selection tree.
 */
void validateSelection(
    const Selection& selection,
    ValidationResult& result
) {
    if (selection.name().empty()) {
        result.addError("Selection name cannot be empty");
    }

    if (selection.alias().has_value() && selection.alias()->empty()) {
        result.addError("Selection alias cannot be empty");
    }

    for (const auto& argument : selection.arguments()) {
        if (argument.name.empty()) {
            result.addError("Argument name cannot be empty");
        }
    }

    for (const auto& child : selection.children()) {
        validateSelection(child, result);
    }
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

Selection& Selection::alias(std::string alias) {
    alias_ = std::move(alias);
    return *this;
}

Selection& Selection::arg(std::string name, Value value) {
    arguments_.push_back({
        .name = std::move(name),
        .value = std::move(value)
    });

    return *this;
}

Selection Selection::fragmentSpread(std::string name) {
    Selection selection(std::move(name));
    selection.kind_ = SelectionKind::FragmentSpread;
    return selection;
}

Selection& Selection::children(std::vector<Selection> children) {
    children_ = std::move(children);
    return *this;
}

std::string Selection::toString(unsigned int indent) const {
    std::ostringstream output;

    if (kind_ == SelectionKind::FragmentSpread) {
        return spaces(indent) + "..." + name_;
    }

    output << spaces(indent);

    if (alias_.has_value()) {
        output << *alias_ << ": ";
    }

    output << name_;
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

Selection spread(std::string name) {
    return Selection::fragmentSpread(std::move(name));
}

const std::string& Selection::name() const {
    return name_;
}

SelectionKind Selection::kind() const {
    return kind_;
}

const std::optional<std::string>& Selection::alias() const {
    return alias_;
}

const std::vector<Argument>& Selection::arguments() const {
    return arguments_;
}

const std::vector<Selection>& Selection::children() const {
    return children_;
}

Fragment::Fragment(
    std::string name,
    std::string typeName,
    std::vector<Selection> selections
)
    : name_(std::move(name)),
      typeName_(std::move(typeName)),
      selections_(std::move(selections)) {
}

std::string Fragment::toString() const {
    std::ostringstream output;

    output << "fragment " << name_ << " on " << typeName_ << " {\n";

    for (const auto& selection : selections_) {
        output << selection.toString(2) << "\n";
    }

    output << "}";

    return output.str();
}

Fragment fragment(
    std::string name,
    std::string typeName,
    std::vector<Selection> selections
) {
    return Fragment(
        std::move(name),
        std::move(typeName),
        std::move(selections)
    );
}

const std::string& Fragment::name() const {
    return name_;
}

const std::string& Fragment::typeName() const {
    return typeName_;
}

const std::vector<Selection>& Fragment::selections() const {
    return selections_;
}

void ValidationResult::addError(std::string error) {
    errors_.push_back(std::move(error));
}

bool ValidationResult::valid() const {
    return errors_.empty();
}

const std::vector<std::string>& ValidationResult::errors() const {
    return errors_;
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

Query& Query::fragment(Fragment fragment) {
    fragments_.push_back(std::move(fragment));
    return *this;
}

ValidationResult Query::validate() const {
    ValidationResult result;

    if (name_.empty()) {
        result.addError("Query name cannot be empty");
    }

    for (const auto& variable : variables_) {
        if (variable.name.empty()) {
            result.addError("Variable name cannot be empty");
        }

        if (variable.type.empty()) {
            result.addError("Variable type cannot be empty");
        }
    }

    for (const auto& selection : selections_) {
        validateSelection(selection, result);
    }

    std::set<std::string> definedFragments;
    std::set<std::string> usedFragments;

    for (const auto& selection : selections_) {
        collectFragmentSpreads(selection, usedFragments);
    }

    for (const auto& fragment : fragments_) {
        if (fragment.name().empty()) {
            result.addError("Fragment name cannot be empty");
        }

        if (fragment.typeName().empty()) {
            result.addError("Fragment type name cannot be empty");
        }

        if (!fragment.name().empty()) {
            definedFragments.insert(fragment.name());
        }

        for (const auto& selection : fragment.selections()) {
            validateSelection(selection, result);
            collectFragmentSpreads(selection, usedFragments);
        }
    }

    for (const auto& used : usedFragments) {
        if (!definedFragments.contains(used)) {
            result.addError("Fragment '" + used + "' is not defined");
        }
    }

    for (const auto& defined : definedFragments) {
        if (!usedFragments.contains(defined)) {
            result.addError("Fragment '" + defined + "' is defined but never used");
        }
    }

    return result;
}

std::string Query::toString() const {
    std::ostringstream output;

    output << "query " << name_ << variablesToString(variables_) << " {\n";

    for (const auto& selection : selections_) {
        output << selection.toString(2) << "\n";
    }

    output << "}";

    for (const auto& fragment : fragments_) {
        output << "\n\n" << fragment.toString();
    }

    return output.str();
}

Query query(std::string name) {
    return Query(std::move(name));
}

} // namespace drogular::gql
