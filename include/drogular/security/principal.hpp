#pragma once

#include <drogular/security/principal_id.hpp>

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace drogular::security {

enum class PrincipalType {
    User,
    Service,
    System
};

class PrincipalBuilder;

// An immutable-by-interface snapshot of an authenticated identity.
// Authentication and authorization decisions are outside this type.
class Principal final {
public:
    Principal(const Principal&) = default;
    Principal(Principal&&) = default;
    Principal& operator=(const Principal&) = default;
    Principal& operator=(Principal&&) = default;

    [[nodiscard]] const PrincipalId& id() const noexcept { return id_; }
    [[nodiscard]] PrincipalType type() const noexcept { return type_; }
    [[nodiscard]] std::string_view displayName() const noexcept { return displayName_; }
    [[nodiscard]] const std::vector<std::string>& roles() const noexcept { return roles_; }

    [[nodiscard]] bool hasRole(std::string_view role) const noexcept {
        return std::any_of(roles_.begin(), roles_.end(),
                           [role](const std::string& candidate) { return candidate == role; });
    }

private:
    friend class PrincipalBuilder;

    Principal(PrincipalId id, PrincipalType type, std::string displayName,
              std::vector<std::string> roles)
        : id_(std::move(id)), type_(type), displayName_(std::move(displayName)),
          roles_(std::move(roles)) {}

    PrincipalId id_;
    PrincipalType type_;
    std::string displayName_;
    std::vector<std::string> roles_;
};

} // namespace drogular::security