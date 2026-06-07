#include <drogular/page.hpp>

namespace drogular {

std::optional<gql::Query> Page::query() const {
    return std::nullopt;
}

} // namespace drogular
