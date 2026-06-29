#pragma once

#include "../portal_project.hpp"

#include <optional>
#include <vector>

class PortalProjectProvider {
public:
    virtual ~PortalProjectProvider() = default;

    virtual std::vector<PortalProject> all() const = 0;

    virtual std::optional<PortalProject> findById(
        int id
    ) const = 0;

    virtual PortalProject create(
        PortalProject project
    ) = 0;

    virtual bool update(
        PortalProject project
    ) = 0;

    virtual bool remove(
        int id
    ) = 0;
};