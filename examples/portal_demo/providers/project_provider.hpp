#pragma once

#include "../data/models/portal_project.hpp"
#include "../data/models/portal_project_create.hpp"
#include "../data/models/portal_project_update.hpp"
#include "../data/models/portal_project_filter.hpp"
#include "../data/models/portal_page.hpp"
#include "../data/models/portal_user.hpp"

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
        const PortalProjectCreate& input,
        int ownerId
    ) = 0;

    virtual PortalProject update(
        const PortalProjectUpdate& input
    ) = 0;

    virtual bool remove(
        int id
    ) = 0;

    virtual std::optional<PortalUser> owner(
        const PortalProject& project
    ) const = 0;

    virtual PortalPage<PortalProject> search(
        const PortalProjectFilter& filter
    ) const = 0;
};