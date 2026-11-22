#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace drogular::security {

using ClaimValue = std::variant<
    bool,
    std::int64_t,
    double, std::string,
    std::vector<std::string>
>;

// Snapshot of values supplied by the caller. Authentication adapters are
// responsible for establishing trust; Claims itself never verifies them.
class Claims final {
public:
    using Storage = std::map<std::string, ClaimValue, std::less<>>;

    [[nodiscard]] bool contains(std::string_view name) const noexcept {
        return values_.find(name) != values_.end();
    }

    [[nodiscard]] const ClaimValue* find(std::string_view name) const noexcept {
        const auto it = values_.find(name);
        return it == values_.end() ? nullptr : &it->second;
    }

    template<class T>
    [[nodiscard]] std::optional<T> get(std::string_view name) const {
        static_assert(std::is_same_v<T, bool> || std::is_same_v<T, std::int64_t> ||
                      std::is_same_v<T, double> || std::is_same_v<T, std::string> ||
                      std::is_same_v<T, std::vector<std::string>>,
                      "Claims::get<T> requires an exact ClaimValue alternative");
        const auto* value = find(name);
        if (value) {
            if (const auto* typed = std::get_if<T>(value)) {
                return *typed;
            }
        }

        return std::nullopt;
    }

    [[nodiscard]] const Storage& values() const noexcept {
        return values_;
    }

private:
    friend class PrincipalBuilder;
    explicit Claims(Storage values) : values_(std::move(values)) {}
    Storage values_;
};

} // namespace drogular::security