#pragma once

#include "features/projects/data/portal_project.hpp"
#include "features/projects/data/portal_project_create.hpp"
#include "features/projects/data/portal_project_update.hpp"
#include "features/projects/data/portal_project_query.hpp"
#include "data/models/portal_page.hpp"
#include "data/models/portal_user.hpp"

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
        const PortalProjectQuery& query
    ) const = 0;
};