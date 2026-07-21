#pragma once

#include "features/projects/data/portal_project.hpp"
#include "features/users/data/portal_user.hpp"
#include "models/portal_role.hpp"
#include "models/portal_project_type.hpp"
#include "features/departments/data/portal_department.hpp"
#include "features/department_members/data/portal_department_member.hpp"

#include <set>
#include <vector>
#include <utility>

class PortalDataset {
public:
    PortalDataset& addRole(
        PortalRole role
    ) {
        roles_.push_back(std::move(role));
        return *this;
    }

    std::vector<PortalRole>& roles() {
        return roles_;
    }

    const std::vector<PortalRole>& roles() const {
        return roles_;
    }

    PortalDataset& addUser(
        PortalUser user
    ) {
        users_.push_back(
            std::move(user)
        );

        return *this;
    }

    PortalDataset& addProject(
        PortalProject project
    ) {
        projects_.push_back(
            std::move(project)
        );

        return *this;
    }

    const std::vector<PortalUser>& users() const {
        return users_;
    }

    std::vector<PortalUser>& users() {
        return users_;
    }

    const std::vector<PortalProject>& projects() const {
        return projects_;
    }

    std::vector<PortalProject>& projects() {
        return projects_;
    }

    PortalDataset& addProjectType(
        PortalProjectType projectType
    ) {
        projectTypes_.push_back(std::move(projectType));
        return *this;
    }

    std::vector<PortalProjectType>& projectTypes() {
        return projectTypes_;
    }

    const std::vector<PortalProjectType>& projectTypes() const {
        return projectTypes_;
    }

    PortalDataset& addDepartment(PortalDepartment value) { departments_.push_back(std::move(value)); return *this; }
    std::vector<PortalDepartment>& departments() { return departments_; }
    const std::vector<PortalDepartment>& departments() const { return departments_; }
    PortalDataset& addDepartmentMember(PortalDepartmentMember value) { departmentMembers_.push_back(std::move(value)); return *this; }
    std::vector<PortalDepartmentMember>& departmentMembers() { return departmentMembers_; }
    const std::vector<PortalDepartmentMember>& departmentMembers() const { return departmentMembers_; }

private:
    std::vector<PortalRole> roles_;
    std::vector<PortalUser> users_;
    std::vector<PortalProject> projects_;
    std::vector<PortalProjectType> projectTypes_;
    std::vector<PortalDepartment> departments_;
    std::vector<PortalDepartmentMember> departmentMembers_;
};