#include <drogular/session.hpp>

#include <utility>

namespace drogular {

void Session::set(
    std::string key,
    std::string value
) {
    values_[std::move(key)] =
        std::move(value);
}

std::optional<std::string> Session::get(
    const std::string& key
) const {
    const auto found =
        values_.find(key);

    if (found == values_.end()) {
        return std::nullopt;
    }

    return found->second;
}

bool Session::has(
    const std::string& key
) const {
    return values_.find(key) != values_.end();
}

void Session::remove(
    const std::string& key
) {
    values_.erase(key);
}

void Session::clear() {
    values_.clear();
}

} // namespace drogular