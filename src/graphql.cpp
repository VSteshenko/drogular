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

} // namespace

Selection::Selection(std::string name)
    : name_(std::move(name)) {
}

Selection::Selection(std::string name, std::vector<Selection> children)
    : name_(std::move(name)),
      children_(std::move(children)) {
}

std::string Selection::toString(unsigned int indent) const {
    std::ostringstream output;

    output << spaces(indent) << name_;

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

Query& Query::select(std::string name, std::vector<Selection> children) {
    selections_.emplace_back(std::move(name), std::move(children));
    return *this;
}

std::string Query::toString() const {
    std::ostringstream output;

    output << "query " << name_ << " {\n";

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
