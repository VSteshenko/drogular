#pragma once

#include <drogular/security/claims.hpp>

namespace drogular::security {

// Application metadata, never implicitly treated as verified identity claims.
class Attributes final {
public:
    using Storage = Claims::Storage;

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
                      "Attributes::get<T> requires an exact ClaimValue alternative");
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
    explicit Attributes(Storage values) : values_(std::move(values)) {}
    Storage values_;
};

} // namespace drogular::security