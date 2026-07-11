#pragma once

#include "../data/models/portal_project_type.hpp"

#include <optional>
#include <vector>

class PortalProjectTypeProvider {
public:
    virtual ~PortalProjectTypeProvider() = default;

    virtual std::vector<PortalProjectType> all() const = 0;

    virtual std::optional<PortalProjectType> findById(
        int id
    ) const = 0;
};
