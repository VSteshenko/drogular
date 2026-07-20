#pragma once

#include "features/department_members/data/portal_department_member.hpp"

#include <optional>
#include <vector>

class PortalDepartmentMemberProvider {
public:
    virtual ~PortalDepartmentMemberProvider() = default;

    virtual std::vector<PortalDepartmentMember> membersOfDepartment(
        int departmentId
    ) const = 0;

    virtual std::vector<PortalDepartmentMember> departmentsOfUser(
        int userId
    ) const = 0;

    virtual std::optional<PortalDepartmentMember> find(
        int departmentId,
        int userId
    ) const = 0;

    virtual PortalDepartmentMember addMember(
        int departmentId,
        int userId
    ) = 0;

    virtual bool removeMember(
        int departmentId,
        int userId
    ) = 0;
};