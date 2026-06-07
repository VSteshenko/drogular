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

Selection::Selection(std::string name, std::vector<std::string> fields)
    : name_(std::move(name)),
      fields_(std::move(fields)) {
}

std::string Selection::toString(unsigned int indent) const {
    std::ostringstream output;

    output << spaces(indent) << name_ << " {\n";

    for (const auto& field : fields_) {
        output << spaces(indent + 2) << field << "\n";
    }

    output << spaces(indent) << "}";

    return output.str();
}

Query::Query(std::string name)
    : name_(std::move(name)) {
}

Query& Query::select(std::string name, std::vector<std::string> fields) {
    selections_.emplace_back(std::move(name), std::move(fields));
    return *this;
}

std::string Query::toString() const {
    std::ostringstream output;

    output << "query " << name_ << " {\n";

    for (const auto& selection : selections_) {
        output << selection.toString(2) << '\n';
    }

    output << "}";

    return output.str();
}

Query query(std::string name) {
    return Query(std::move(name));
}

} // namespace drogular::gql
