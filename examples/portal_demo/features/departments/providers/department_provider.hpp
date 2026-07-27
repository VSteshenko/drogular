#pragma once

#include "features/departments/data/portal_department.hpp"
#include "features/departments/data/portal_department_create.hpp"
#include "features/departments/data/portal_department_update.hpp"
#include "features/departments/data/portal_department_query.hpp"

#include <drogular/paged_result.hpp>

#include <optional>
#include <string>
#include <vector>

class PortalDepartmentProvider {
public:
    virtual ~PortalDepartmentProvider() = default;

    virtual std::vector<PortalDepartment> all() const = 0;

    virtual drogular::PagedResult<PortalDepartment> search(
        const PortalDepartmentQuery&
    ) const = 0;

    virtual std::optional<PortalDepartment> findById(
        int id
    ) const = 0;

    virtual bool exists(
        const std::string& name,
        std::optional<int> excludingId = std::nullopt
    ) const = 0;

    virtual PortalDepartment create(
        const PortalDepartmentCreate&
    ) = 0;

    virtual PortalDepartment update(
        const PortalDepartmentUpdate&
    ) = 0;
};