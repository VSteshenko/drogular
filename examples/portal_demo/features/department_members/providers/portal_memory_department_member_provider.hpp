#pragma once

#include "features/department_members/providers/department_member_provider.hpp"

#include <algorithm>

class PortalMemoryDepartmentMemberProvider final
    : public PortalDepartmentMemberProvider
{
public:
    PortalMemoryDepartmentMemberProvider()
        : members_({
            {1, 1, 1},
            {2, 1, 2},
            {3, 2, 2},
            {4, 2, 3}
        }), nextId_(5)
    {
    }

    explicit PortalMemoryDepartmentMemberProvider(
        std::vector<PortalDepartmentMember> members
    )
        : members_(std::move(members))
    {
        for (const auto& member : members_) {
            nextId_ = std::max(nextId_, member.id + 1);
        }
    }

    std::vector<PortalDepartmentMember> membersOfDepartment(
        int departmentId
    ) const override {
        std::vector<PortalDepartmentMember> result;
        for (const auto& member : members_) {
            if (member.departmentId == departmentId) {
                result.push_back(member);
            }
        }
        return result;
    }

    std::vector<PortalDepartmentMember> departmentsOfUser(
        int userId
    ) const override {
        std::vector<PortalDepartmentMember> result;
        for (const auto& member : members_) {
            if (member.userId == userId) {
                result.push_back(member);
            }
        }
        return result;
    }

    std::optional<PortalDepartmentMember> find(
        int departmentId,
        int userId
    ) const override {
        for (const auto& member : members_) {
            if (member.departmentId == departmentId && member.userId == userId) {
                return member;
            }
        }
        return std::nullopt;
    }

    PortalDepartmentMember addMember(
        int departmentId,
        int userId
    ) override {
        if (const auto existing =
            find(departmentId, userId)
        ) {
            return *existing;
        }

        PortalDepartmentMember member{nextId_++, departmentId, userId};
        members_.push_back(member);
        return member;
    }

    bool removeMember(
        int departmentId,
        int userId
    ) override {
        const auto before = members_.size();
        std::erase_if(
            members_,
            [=](const auto& member) {
            return member.departmentId == departmentId &&
                member.userId == userId;
        });
        return members_.size() != before;
    }

private:
    std::vector<PortalDepartmentMember> members_;
    int nextId_ = 1;
};