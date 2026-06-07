#include <drogular/graphql.hpp>

#include <utility>

namespace drogular::gql {

Query::Query(std::string name)
    : name_(std::move(name)) {
}

std::string Query::toString() const {
    return "query " + name_ + " {\n}";
}

} // namespace drogular::gql
