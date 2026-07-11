#pragma once

#include "../data/models/portal_user.hpp"

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

    virtual bool update(
        PortalUser user
    ) = 0;

    virtual std::optional<PortalUser> findById(
        int id
    ) const = 0;

    virtual std::optional<PortalUser> findByUsername(
        const std::string& username
    ) const = 0;

};