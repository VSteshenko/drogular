#pragma once

#include "../portal_project.hpp"
#include "../portal_user.hpp"

#include <utility>
#include <vector>

class PortalDataset {
public:
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

    std::vector<PortalUser>& users()
    {
        return users_;
    }

    const std::vector<PortalProject>& projects() const {
        return projects_;
    }

    std::vector<PortalProject>& projects() {
        return projects_;
    }

private:
    std::vector<PortalUser> users_;
    std::vector<PortalProject> projects_;
};