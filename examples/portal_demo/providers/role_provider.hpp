#pragma once

#include "data/models/portal_role.hpp"
#include "data/models/portal_role_create.hpp"
#include "data/models/portal_role_update.hpp"

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

    virtual std::optional<PortalRole> findById(
        int id
    ) const = 0;

    virtual PortalRole create(
        const PortalRoleCreate& input
    ) = 0;

    virtual PortalRole update(
        const PortalRoleUpdate& input
    ) = 0;

    virtual bool remove(
        int id
    ) = 0;
};