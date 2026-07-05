#pragma once

#include "../portal_role.hpp"

#include <optional>
#include <string>
#include <vector>

class PortalRoleProvider {
public:
    virtual ~PortalRoleProvider() = default;

    virtual std::vector<PortalRole> all() const = 0;

    virtual std::optional<PortalRole> findByCode(
        const std::string& code
    ) const = 0;
};