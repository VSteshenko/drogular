#pragma once

#include <string>

namespace drogular::security {

// An identity is scoped by its issuer; subject alone is not globally unique.
struct PrincipalId final {
    std::string issuer;
    std::string subject;

    bool operator==(const PrincipalId&) const = default;
};

} // namespace drogular::security