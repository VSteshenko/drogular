#pragma once

#include "portal_user.hpp"

#include <optional>
#include <string>
#include <vector>

class PortalUserProvider {
public:
    virtual ~PortalUserProvider() = default;

    virtual std::vector<PortalUser> all() const = 0;

    virtual std::optional<PortalUser> findByCredentials(
        const std::string& username,
        const std::string& password
    ) const = 0;

    virtual bool exists(
        const std::string& username
    ) const = 0;

    virtual PortalUser create(
        PortalUser user
    ) = 0;
};