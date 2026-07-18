#pragma once

#include "features/projects/data/portal_project.hpp"
#include "features/users/data/portal_user.hpp"
#include "models/portal_role.hpp"
#include "models/portal_project_type.hpp"

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

private:
    std::vector<PortalRole> roles_;
    std::vector<PortalUser> users_;
    std::vector<PortalProject> projects_;
    std::vector<PortalProjectType> projectTypes_;
};