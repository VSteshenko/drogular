#pragma once

#include "features/project_types/data/portal_project_type.hpp"

#include <optional>
#include <vector>
#include "features/project_types/data/portal_project_type_create.hpp"
#include "features/project_types/data/portal_project_type_update.hpp"

class PortalProjectTypeProvider {
public:
    virtual ~PortalProjectTypeProvider() = default;

    virtual std::vector<PortalProjectType> all() const = 0;

    virtual std::optional<PortalProjectType> findById(
        int id
    ) const = 0;

    virtual PortalProjectType create(
        const PortalProjectTypeCreate& input
    ) = 0;

    virtual PortalProjectType update(
        const PortalProjectTypeUpdate& input
    ) = 0;

    virtual bool remove(
        int id
    ) = 0;
};